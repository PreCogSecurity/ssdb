/*
 * SSDB Core Tests
 * Copyright (c) 2024 PreCog Security
 */

#include "test_harness.h"
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <thread>
#include <chrono>

// Include SSDB headers
extern "C" {
#include "../src/util/log.h"
#include "../src/util/config.h"
#include "../src/ssdb/ssdb.h"
#include "../src/ssdb/options.h"
#include "../src/ssdb/iterator.h"
}

using namespace ssdb_test;

// Helper to start SSDB server in background
class SSDBServer {
public:
    SSDBServer(int port = 18888) : port_(port), pid_(-1) {}

    ~SSDBServer() {
        Stop();
    }

    bool Start(const std::string& work_dir = "./test_var") {
        // Create work directory
        std::string cmd = "mkdir -p " + work_dir;
        system(cmd.c_str());

        // Write config file
        std::string config = 
            "work_dir = " + work_dir + "\n"
            "pidfile = " + work_dir + "/ssdb.pid\n"
            "server:\n"
            "    ip: 127.0.0.1\n"
            "    port: " + std::to_string(port_) + "\n"
            "logger:\n"
            "    level: error\n"
            "    output: " + work_dir + "/log.txt\n"
            "leveldb:\n"
            "    cache_size: 100\n"
            "    block_size: 32\n"
            "    write_buffer_size: 64\n"
            "    compression: yes\n";

        std::string config_file = work_dir + "/ssdb_test.conf";
        FILE* fp = fopen(config_file.c_str(), "w");
        if (!fp) return false;
        fwrite(config.c_str(), 1, config.size(), fp);
        fclose(fp);

        // Start server
        pid_ = fork();
        if (pid_ == 0) {
            // Child process
            execl("./ssdb-server", "ssdb-server", config_file.c_str(), (char*)NULL);
            exit(1);
        } else if (pid_ > 0) {
            // Parent process - wait for server to start
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return IsRunning();
        }
        return false;
    }

    void Stop() {
        if (pid_ > 0) {
            kill(pid_, SIGTERM);
            waitpid(pid_, NULL, 0);
            pid_ = -1;
        }
    }

    bool IsRunning() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return false;

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        close(sock);
        return result == 0;
    }

    int GetPort() const { return port_; }

private:
    int port_;
    pid_t pid_;
};

// Simple SSDB client for testing
class TestClient {
public:
    TestClient(int port = 18888) : port_(port), sock_(-1) {}

    ~TestClient() {
        Disconnect();
    }

    bool Connect() {
        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) return false;

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock_);
            sock_ = -1;
            return false;
        }
        return true;
    }

    void Disconnect() {
        if (sock_ >= 0) {
            close(sock_);
            sock_ = -1;
        }
    }

    // Send RESP-style command and read response
    std::vector<std::string> SendCommand(const std::vector<std::string>& args) {
        std::string cmd = "*" + std::to_string(args.size()) + "\r\n";
        for (const auto& arg : args) {
            cmd += "$" + std::to_string(arg.size()) + "\r\n";
            cmd += arg + "\r\n";
        }

        if (send(sock_, cmd.c_str(), cmd.size(), 0) < 0) {
            return {"error", "send failed"};
        }

        return ReadResponse();
    }

    std::vector<std::string> ReadResponse() {
        std::vector<std::string> result;
        char buf[4096];
        std::string buffer;

        while (true) {
            int n = recv(sock_, buf, sizeof(buf) - 1, 0);
            if (n <= 0) break;
            buf[n] = '\0';
            buffer += buf;

            // Try to parse complete response
            auto parsed = ParseRESP(buffer);
            if (!parsed.empty()) {
                result = parsed;
                break;
            }
        }
        return result;
    }

private:
    std::vector<std::string> ParseRESP(const std::string& data) {
        std::vector<std::string> result;
        size_t pos = 0;

        if (data.empty()) return result;

        if (data[0] == '*') {
            // Array
            size_t end = data.find("\r\n", pos);
            if (end == std::string::npos) return result;
            int count = std::stoi(data.substr(1, end - 1));
            pos = end + 2;

            for (int i = 0; i < count; i++) {
                if (pos >= data.size()) return {};
                if (data[pos] != '$') return {};
                pos++;
                end = data.find("\r\n", pos);
                if (end == std::string::npos) return {};
                int len = std::stoi(data.substr(pos, end - pos));
                pos = end + 2;
                if (pos + len + 2 > data.size()) return {};
                result.push_back(data.substr(pos, len));
                pos += len + 2;
            }
        } else if (data[0] == '+') {
            // Simple string
            size_t end = data.find("\r\n", pos);
            if (end == std::string::npos) return result;
            result.push_back(data.substr(1, end - 1));
        } else if (data[0] == '-') {
            // Error
            size_t end = data.find("\r\n", pos);
            if (end == std::string::npos) return result;
            result.push_back("error");
            result.push_back(data.substr(1, end - 1));
        } else if (data[0] == ':') {
            // Integer
            size_t end = data.find("\r\n", pos);
            if (end == std::string::npos) return result;
            result.push_back(data.substr(1, end - 1));
        } else if (data[0] == '$') {
            // Bulk string
            size_t end = data.find("\r\n", pos);
            if (end == std::string::npos) return result;
            int len = std::stoi(data.substr(1, end - 1));
            pos = end + 2;
            if (pos + len + 2 > data.size()) return {};
            result.push_back(data.substr(pos, len));
        }

        return result;
    }

    int port_;
    int sock_;
};

// Test basic KV operations
TEST(KV_BasicSetGet) {
    SSDBServer server(18888);
    ASSERT_TRUE(server.Start("./test_var_kv"));

    TestClient client(18888);
    ASSERT_TRUE(client.Connect());

    // SET
    auto resp = client.SendCommand({"set", "key1", "value1"});
    ASSERT_EQ("ok", resp[0]);

    // GET
    resp = client.SendCommand({"get", "key1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("value1", resp[1]);

    // GET non-existent
    resp = client.SendCommand({"get", "nonexistent"});
    ASSERT_EQ("not_found", resp[0]);

    // DEL
    resp = client.SendCommand({"del", "key1"});
    ASSERT_EQ("ok", resp[0]);

    // GET after DEL
    resp = client.SendCommand({"get", "key1"});
    ASSERT_EQ("not_found", resp[0]);

    server.Stop();
}

// Test INCR/DECR
TEST(KV_IncrDecr) {
    SSDBServer server(18889);
    ASSERT_TRUE(server.Start("./test_var_incr"));

    TestClient client(18889);
    ASSERT_TRUE(client.Connect());

    // INCR new key
    auto resp = client.SendCommand({"incr", "counter"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    // INCR existing
    resp = client.SendCommand({"incr", "counter"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("2", resp[1]);

    // INCR by 5
    resp = client.SendCommand({"incr", "counter", "5"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("7", resp[1]);

    // DECR
    resp = client.SendCommand({"decr", "counter"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("6", resp[1]);

    // DECR by 3
    resp = client.SendCommand({"decr", "counter", "3"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("3", resp[1]);

    server.Stop();
}

// Test Hash operations
TEST(Hash_Basic) {
    SSDBServer server(18890);
    ASSERT_TRUE(server.Start("./test_var_hash"));

    TestClient client(18890);
    ASSERT_TRUE(client.Connect());

    // HSET
    auto resp = client.SendCommand({"hset", "myhash", "field1", "value1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    // HGET
    resp = client.SendCommand({"hget", "myhash", "field1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("value1", resp[1]);

    // HGET non-existent
    resp = client.SendCommand({"hget", "myhash", "nonexistent"});
    ASSERT_EQ("not_found", resp[0]);

    // HDEL
    resp = client.SendCommand({"hdel", "myhash", "field1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    // HGET after HDEL
    resp = client.SendCommand({"hget", "myhash", "field1"});
    ASSERT_EQ("not_found", resp[0]);

    // HSIZE
    resp = client.SendCommand({"hsize", "myhash"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("0", resp[1]);

    server.Stop();
}

// Test ZSet operations
TEST(ZSet_Basic) {
    SSDBServer server(18891);
    ASSERT_TRUE(server.Start("./test_var_zset"));

    TestClient client(18891);
    ASSERT_TRUE(client.Connect());

    // ZSET
    auto resp = client.SendCommand({"zset", "myzset", "member1", "10"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    // ZGET
    resp = client.SendCommand({"zget", "myzset", "member1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("10", resp[1]);

    // ZSET another
    resp = client.SendCommand({"zset", "myzset", "member2", "20"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    // ZSIZE
    resp = client.SendCommand({"zsize", "myzset"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("2", resp[1]);

    // ZRANGE
    resp = client.SendCommand({"zrange", "myzset", "0", "10"});
    ASSERT_EQ("ok", resp[0]);
    // Should have member1 and member2
    ASSERT_TRUE(resp.size() >= 4); // ok, member1, score1, member2, score2...

    // ZDEL
    resp = client.SendCommand({"zdel", "myzset", "member1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    server.Stop();
}

// Test Queue operations
TEST(Queue_Basic) {
    SSDBServer server(18892);
    ASSERT_TRUE(server.Start("./test_var_queue"));

    TestClient client(18892);
    ASSERT_TRUE(client.Connect());

    // QPUSH_FRONT
    auto resp = client.SendCommand({"qpush_front", "myqueue", "item1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    // QPUSH_BACK
    resp = client.SendCommand({"qpush_back", "myqueue", "item2"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("2", resp[1]);

    // QSIZE
    resp = client.SendCommand({"qsize", "myqueue"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("2", resp[1]);

    // QFRONT
    resp = client.SendCommand({"qfront", "myqueue"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("item1", resp[1]);

    // QBACK
    resp = client.SendCommand({"qback", "myqueue"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("item2", resp[1]);

    // QPOP_FRONT
    resp = client.SendCommand({"qpop_front", "myqueue"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("item1", resp[1]);

    // QPOP_BACK
    resp = client.SendCommand({"qpop_back", "myqueue"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("item2", resp[1]);

    // QSIZE after pops
    resp = client.SendCommand({"qsize", "myqueue"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("0", resp[1]);

    server.Stop();
}

// Test SCAN operations
TEST(KV_Scan) {
    SSDBServer server(18893);
    ASSERT_TRUE(server.Start("./test_var_scan"));

    TestClient client(18893);
    ASSERT_TRUE(client.Connect());

    // Insert multiple keys
    for (int i = 0; i < 10; i++) {
        std::string key = "key" + std::to_string(i);
        std::string val = "value" + std::to_string(i);
        auto resp = client.SendCommand({"set", key, val});
        ASSERT_EQ("ok", resp[0]);
    }

    // SCAN
    auto resp = client.SendCommand({"scan", "", "", "100"});
    ASSERT_EQ("ok", resp[0]);
    // Should have 10 key-value pairs = 20 elements + "ok"
    ASSERT_TRUE(resp.size() >= 21);

    // RSCAN
    resp = client.SendCommand({"rscan", "", "", "100"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_TRUE(resp.size() >= 21);

    server.Stop();
}

// Test TTL operations
TEST(KV_TTL) {
    SSDBServer server(18894);
    ASSERT_TRUE(server.Start("./test_var_ttl"));

    TestClient client(18894);
    ASSERT_TRUE(client.Connect());

    // SETX with TTL
    auto resp = client.SendCommand({"setx", "ttlkey", "ttlvalue", "1"});
    ASSERT_EQ("ok", resp[0]);

    // GET immediately
    resp = client.SendCommand({"get", "ttlkey"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("ttlvalue", resp[1]);

    // TTL
    resp = client.SendCommand({"ttl", "ttlkey"});
    ASSERT_EQ("ok", resp[0]);
    // TTL should be > 0
    int ttl = std::stoi(resp[1]);
    ASSERT_TRUE(ttl > 0 && ttl <= 1);

    // Wait for expiry
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // GET after expiry
    resp = client.SendCommand({"get", "ttlkey"});
    ASSERT_EQ("not_found", resp[0]);

    server.Stop();
}

// Test multi operations
TEST(KV_Multi) {
    SSDBServer server(18895);
    ASSERT_TRUE(server.Start("./test_var_multi"));

    TestClient client(18895);
    ASSERT_TRUE(client.Connect());

    // MULTI_SET
    auto resp = client.SendCommand({"multi_set", "k1", "v1", "k2", "v2", "k3", "v3"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("3", resp[1]);

    // MULTI_GET
    resp = client.SendCommand({"multi_get", "k1", "k2", "k3", "k4"});
    ASSERT_EQ("ok", resp[0]);
    // Should have k1,v1,k2,v2,k3,v3 (k4 not found)
    ASSERT_TRUE(resp.size() >= 7);

    // MULTI_DEL
    resp = client.SendCommand({"multi_del", "k1", "k2"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("2", resp[1]);

    // Verify deleted
    resp = client.SendCommand({"multi_get", "k1", "k2", "k3"});
    ASSERT_EQ("ok", resp[0]);
    // Should only have k3,v3
    ASSERT_TRUE(resp.size() >= 3);

    server.Stop();
}

// Test bit operations
TEST(KV_BitOps) {
    SSDBServer server(18896);
    ASSERT_TRUE(server.Start("./test_var_bit"));

    TestClient client(18896);
    ASSERT_TRUE(client.Connect());

    // SETBIT
    auto resp = client.SendCommand({"setbit", "bits", "0", "1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    resp = client.SendCommand({"setbit", "bits", "1", "1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    resp = client.SendCommand({"setbit", "bits", "7", "1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    // GETBIT
    resp = client.SendCommand({"getbit", "bits", "0"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    resp = client.SendCommand({"getbit", "bits", "1"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("1", resp[1]);

    resp = client.SendCommand({"getbit", "bits", "2"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("0", resp[1]);

    // BITCOUNT
    resp = client.SendCommand({"bitcount", "bits"});
    ASSERT_EQ("ok", resp[0]);
    ASSERT_EQ("3", resp[1]);

    server.Stop();
}

// Test error handling
TEST(ErrorHandling) {
    SSDBServer server(18897);
    ASSERT_TRUE(server.Start("./test_var_error"));

    TestClient client(18897);
    ASSERT_TRUE(client.Connect());

    // Wrong number of arguments
    auto resp = client.SendCommand({"set"});
    ASSERT_EQ("client_error", resp[0]);

    resp = client.SendCommand({"get"});
    ASSERT_EQ("client_error", resp[0]);

    // Invalid bit offset
    resp = client.SendCommand({"setbit", "key", "-1", "1"});
    ASSERT_EQ("client_error", resp[0]);

    resp = client.SendCommand({"setbit", "key", "1000000000", "1"});
    ASSERT_EQ("client_error", resp[0]);

    // Invalid bit value
    resp = client.SendCommand({"setbit", "key", "0", "2"});
    ASSERT_EQ("client_error", resp[0]);

    server.Stop();
}

// Main entry point
int main(int argc, char** argv) {
    // Initialize logging
    set_log_level(Logger::LEVEL_ERROR);

    // Run all tests
    return TestHarness::Instance().RunAll();
}