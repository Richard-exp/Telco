#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <deque>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

struct Config {
    uint16_t ingest_port = 9000;
    std::string oss_host = "127.0.0.1";
    uint16_t oss_port = 9100;
    std::size_t max_buffer = 10000;
};

struct TelemetryRecord {
    std::chrono::system_clock::time_point event_time;
    std::chrono::system_clock::time_point processing_time;
    std::string payload;
};

std::atomic<bool> g_running = true;

std::string toMillisString(const std::chrono::system_clock::time_point& tp) {
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
    return std::to_string(ms);
}

bool parseArgs(int argc, char** argv, Config& cfg) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto requireValue = [&](const std::string& flag) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << flag << "\n";
                return nullptr;
            }
            return argv[++i];
        };

        if (arg == "--listen-port") {
            if (const char* value = requireValue(arg)) {
                cfg.ingest_port = static_cast<uint16_t>(std::stoi(value));
            } else {
                return false;
            }
        } else if (arg == "--oss-host") {
            if (const char* value = requireValue(arg)) {
                cfg.oss_host = value;
            } else {
                return false;
            }
        } else if (arg == "--oss-port") {
            if (const char* value = requireValue(arg)) {
                cfg.oss_port = static_cast<uint16_t>(std::stoi(value));
            } else {
                return false;
            }
        } else if (arg == "--max-buffer") {
            if (const char* value = requireValue(arg)) {
                cfg.max_buffer = static_cast<std::size_t>(std::stoull(value));
                if (cfg.max_buffer == 0) {
                    std::cerr << "--max-buffer must be greater than zero\n";
                    return false;
                }
            } else {
                return false;
            }
        } else if (arg == "--help") {
            std::cout << "Usage: gateway_daemon [--listen-port N] [--oss-host HOST] [--oss-port N] [--max-buffer N]\n";
            return false;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return false;
        }
    }
    return true;
}

int createServerSocket(uint16_t port) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    if (listen(fd, 8) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

int connectToOss(const std::string& host, uint16_t port) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    const std::string port_str = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result) != 0) {
        return -1;
    }

    int sock = -1;
    for (addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0) {
            continue;
        }
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        close(sock);
        sock = -1;
    }

    freeaddrinfo(result);
    return sock;
}

bool sendAll(int sock, const std::string& data) {
    std::size_t sent_total = 0;
    while (sent_total < data.size() && g_running) {
        const ssize_t sent = send(sock, data.data() + sent_total, data.size() - sent_total, 0);
        if (sent <= 0) {
            return false;
        }
        sent_total += static_cast<std::size_t>(sent);
    }
    return sent_total == data.size();
}

TelemetryRecord buildRecord(const std::string& payload) {
    const auto now = std::chrono::system_clock::now();
    const auto separator = payload.find('|');
    if (separator != std::string::npos) {
        const std::string possible_ts = payload.substr(0, separator);
        try {
            const auto event_ms = std::stoll(possible_ts);
            TelemetryRecord rec{};
            rec.event_time = std::chrono::system_clock::time_point(std::chrono::milliseconds(event_ms));
            rec.processing_time = now;
            rec.payload = payload.substr(separator + 1);
            return rec;
        } catch (...) {
        }
    }

    TelemetryRecord rec{};
    rec.event_time = now;
    rec.processing_time = now;
    rec.payload = payload;
    return rec;
}

void handleSignal(int) {
    g_running = false;
}

class TelemetryBuffer {
   public:
    explicit TelemetryBuffer(std::size_t max_size) : max_size_(max_size) {}

    bool enqueue(TelemetryRecord record) {
        std::unique_lock<std::mutex> lock(mutex_);
        can_push_.wait(lock, [&] { return queue_.size() < max_size_ || !g_running; });
        if (!g_running) {
            return false;
        }
        queue_.push_back(std::move(record));
        can_pop_.notify_one();
        return true;
    }

    std::optional<TelemetryRecord> dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);
        can_pop_.wait(lock, [&] { return !queue_.empty() || !g_running; });
        if (queue_.empty()) {
            return std::nullopt;
        }
        TelemetryRecord record = std::move(queue_.front());
        queue_.pop_front();
        can_push_.notify_one();
        return record;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        can_push_.notify_all();
        can_pop_.notify_all();
    }

   private:
    std::size_t max_size_;
    std::deque<TelemetryRecord> queue_;
    std::mutex mutex_;
    std::condition_variable can_push_;
    std::condition_variable can_pop_;
};

}  // namespace

int main(int argc, char** argv) {
    Config config{};
    if (!parseArgs(argc, argv, config)) {
        return 1;
    }

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    TelemetryBuffer buffer(config.max_buffer);

    std::thread sender([&] {
        int oss_sock = -1;
        auto backoff = std::chrono::milliseconds(100);
        const auto max_backoff = std::chrono::milliseconds(5000);

        while (g_running) {
            if (oss_sock < 0) {
                oss_sock = connectToOss(config.oss_host, config.oss_port);
                if (oss_sock < 0) {
                    std::this_thread::sleep_for(backoff);
                    backoff = std::min(backoff * 2, max_backoff);
                    continue;
                }
                backoff = std::chrono::milliseconds(100);
            }

            auto rec = buffer.dequeue();
            if (!rec.has_value()) {
                break;
            }

            const std::string outbound =
                toMillisString(rec->event_time) + "|" +
                toMillisString(rec->processing_time) + "|" +
                rec->payload + "\n";

            if (!sendAll(oss_sock, outbound)) {
                close(oss_sock);
                oss_sock = -1;
            }
        }

        if (oss_sock >= 0) {
            close(oss_sock);
        }
    });

    const int server_fd = createServerSocket(config.ingest_port);
    if (server_fd < 0) {
        std::cerr << "Failed to create ingest server on port " << config.ingest_port << "\n";
        g_running = false;
        buffer.shutdown();
        sender.join();
        return 1;
    }

    std::cout << "Gateway daemon listening on port " << config.ingest_port
              << " and forwarding to " << config.oss_host << ":" << config.oss_port << "\n";

    while (g_running) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        const int client = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client < 0) {
            if (!g_running) {
                break;
            }
            continue;
        }

        std::string pending;
        std::vector<char> recv_buf(4096);

        while (g_running) {
            const ssize_t n = recv(client, recv_buf.data(), recv_buf.size(), 0);
            if (n <= 0) {
                break;
            }
            pending.append(recv_buf.data(), static_cast<std::size_t>(n));

            std::size_t line_end = 0;
            while ((line_end = pending.find('\n')) != std::string::npos) {
                std::string line = pending.substr(0, line_end);
                pending.erase(0, line_end + 1);
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                if (line.empty()) {
                    continue;
                }
                if (!buffer.enqueue(buildRecord(line))) {
                    break;
                }
            }
        }

        close(client);
    }

    close(server_fd);
    g_running = false;
    buffer.shutdown();
    sender.join();
    return 0;
}
