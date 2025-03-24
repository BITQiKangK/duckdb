#include "duckdb/llm/llm_call.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace duckdb {
std::string api_key = "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";

// 回调函数用于接收HTTP响应数据
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

// 去除字符串两端的空白字符（类似Python的strip函数）
std::string strip(const std::string& str);

// 使用curl发送请求
bool send_request(const std::string& request_body, std::string& response);

// Below are llm_call function serise
// 单轮LLM调用
std::string single_round_llm_call(const std::string& prompt) {
    // 准备JSON请求体
    nlohmann::json request_body = {
        {"model", "google/gemma-3-12b-it:free"},
        {"messages", {{
            {"role", "user"},
            {"content", prompt}
        }}}
    };

    std::string response;
    send_request(request_body.dump(), response);
    return response;
}


// Below are helper functions
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string strip(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

bool send_request(const std::string& request_body, std::string& response) {
    CURL* curl = curl_easy_init();
    std::string curl_response;

    if(curl) {
        // 设置请求头和URL
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        curl_easy_setopt(curl, CURLOPT_URL, "https://openrouter.ai/api/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_response);

        // 添加重定向处理选项
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
        
        // 发送请求 - 最多尝试3次
        CURLcode res = CURLE_OK;
        int max_attempts = 3;
        
        for (int attempt = 0; attempt < max_attempts; attempt++) {
            res = curl_easy_perform(curl);
            
            if (res == CURLE_OK) {
                // 请求成功，跳出循环
                break;
            }
        }
        
        // 清理
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    // 解析响应JSON并提取回复内容
    try {
        auto json_response = nlohmann::json::parse(curl_response);
        response = strip(json_response["choices"][0]["message"]["content"]);
        return true;
    } catch(const std::exception& e) {
        response = "Error parsing response: " + std::string(e.what());
        return false;
    }
}

}