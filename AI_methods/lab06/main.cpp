#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "llama.h"
#include "common.h"

int main(int argc, char ** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_model.gguf>" << std::endl;
        return 1;
    }
    const std::string model_path = argv[1];

    llama_backend_init();
    
    auto model_params = llama_model_default_params();
    llama_model* model = llama_load_model_from_file(model_path.c_str(), model_params);
    if (!model) {
        std::cerr << "Error: Could not load model!" << std::endl;
        return 1;
    }

    auto ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;
    llama_context* ctx = llama_new_context_with_model(model, ctx_params);
    if (!ctx) {
        std::cerr << "Error: Could not create context!" << std::endl;
        return 1;
    }

    std::string system_prompt = 
        "You are an expert C++ developer. Your task is to write high-quality, modern C++ code. "
        "Provide only the code, no explanations, unless asked for a detailed review. "
        "Use 'const', 'auto', and smart pointers where appropriate.";
        
    std::string user_query = "Write a C++ function that takes a vector of integers and returns a new vector containing only the unique elements, preserving the original order.";

    std::vector<llama_chat_message> messages;
    messages.push_back({"system", system_prompt.c_str()});
    messages.push_back({"user", user_query.c_str()});

    // Применяем встроенный Jinja-шаблон модели для правильной расстановки специальных токенов
    std::string formatted_prompt;
    int num_chars = llama_chat_apply_template(model, nullptr, messages.data(), messages.size(), true, nullptr, 0);
    if (num_chars > 0) {
        formatted_prompt.resize(num_chars);
        llama_chat_apply_template(model, nullptr, messages.data(), messages.size(), true, formatted_prompt.data(), formatted_prompt.size());
    } else {
        std::cerr << "Warning: Could not apply chat template, using raw prompt." << std::endl;
        formatted_prompt = user_query;
    }

    // 3. ТОКЕНИЗАЦИЯ (Превращаем текст в числа для модели)
    std::vector<llama_token> tokens_list;
    tokens_list = common_tokenize(ctx, formatted_prompt, true); // true = добавить BOS токен

    // 4. НАСТРОЙКА СЕМПЛИНГА (Как модель выбирает слова)
    auto sparams = llama_sampler_chain_default_params();
    llama_sampler* sampler = llama_sampler_chain_init(sparams);
    // Температура = 0.7 (немного креативности для кода)
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.7f));
    // Top-p = 0.9
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(0.9f, 1));
    // Min-p = 0.05 (убирает совсем маловероятные токены)
    llama_sampler_chain_add(sampler, llama_sampler_init_min_p(0.05f, 1));
    // Сэмплер дистрибуции
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    std::cout << "🤖 Generating code for: '" << user_query << "'\n" << std::endl;
    std::cout << "--- START OF GENERATED CODE ---" << std::endl;

    llama_batch batch = llama_batch_init(1, 0, 1);
    
    for (size_t i = 0; i < tokens_list.size(); i++) {
        common_batch_add(batch, tokens_list[i], i, {0}, false);
    }
    
    int n_predict = 512;
    int n_cur = batch.n_tokens;
    
    while (n_cur <= n_predict) {
       
        if (llama_decode(ctx, batch)) {
            std::cerr << "\nError: llama_decode failed!" << std::endl;
            break;
        }
        
        llama_token new_token_id = llama_sampler_sample(sampler, ctx, -1);
        
        if (llama_token_is_eog(model, new_token_id)) {
            break;
        }
        
        std::string token_str = common_token_to_piece(ctx, new_token_id);
        std::cout << token_str << std::flush;
        
        common_batch_clear(batch);
        common_batch_add(batch, new_token_id, n_cur, {0}, true);
        n_cur++;
    }
    
    std::cout << "\n--- END OF GENERATED CODE ---" << std::endl;

    // memmory cleaner
    llama_batch_free(batch);
    llama_sampler_free(sampler);
    llama_free(ctx);
    llama_free_model(model);
    llama_backend_free();
    
    return 0;
}