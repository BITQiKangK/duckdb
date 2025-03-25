#include "duckdb/function/scalar/string_functions.hpp"
#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/common/vector_operations/unary_executor.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckdb/llm/llm_call.hpp"
#include <string>

namespace duckdb {

// 实现sem_filter函数的操作
struct SemFilterOperator {
    template <class INPUT_TYPE, class RESULT_TYPE>
    static RESULT_TYPE Operation(INPUT_TYPE input) {
        auto input_data = input.GetData();
        auto input_length = input.GetSize();
        
        std::string prompt = "Please determine if the following string is a valid semantic entity: " + 
                              std::string(input_data, input_length) + 
                              " Please only return true or false.";
        
        std::string llm_response = single_round_llm_call(prompt);
        
        // 将LLM响应转换为布尔值
        // 默认为false
        bool filter_result = false;
        
        // 转换为小写进行比较
        std::transform(llm_response.begin(), llm_response.end(), llm_response.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        
        // 检查是否包含表示真的关键词
        if (llm_response.find("true") != std::string::npos) {
            filter_result = true;
        }
        
        return RESULT_TYPE(filter_result);
    }
};

// 注册sem_filter函数
static void SemFilterFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    UnaryExecutor::Execute<string_t, bool, SemFilterOperator>(args.data[0], result, args.size());
}

// 获取函数定义
ScalarFunction SemFilterFun::GetFunction() {
    return ScalarFunction("sem_filter", {LogicalType::VARCHAR}, LogicalType::BOOLEAN, SemFilterFunction);
}

} // namespace duckdb 