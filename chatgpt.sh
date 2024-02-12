#!/bin/bash
organization_id=org-FsoycnugomBaekWQ1fctGtn4
api_token=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
# Function to list available models
list_models() {
    api_token=$1
    echo -e "\e[31musing api token $api_token\e[0m"
    curl -sS -H "Authorization: Bearer $api_token" -H "Openai-Organization: $organization_id" https://api.openai.com/v1/engines | jq '.data[] | .id'
}

# Function to chat with GPT
chat() {
    local api_token=$1
    local model=$2
    local conversation_file="$3"
    echo -e "\e[31musing api token $api_token, model $model. Storing conversation in $conversation_file\e[0m"

    while true; do
        echo -e "\e[32m[$model][$conversation_file] User:\e[0m"
        read -p "> " question
        
        if [[ $question == "exit" ]]; then
            break
        fi
        
        echo "$model|$conversation_file|User: $question" >> ~/.chatgpt/$conversation_file
        full_response=$(curl -sS -X POST \
                -H "Authorization: Bearer $api_token" \
                -H "Openai-Organization: $organization_id" \
                -H 'Content-Type: application/json' \
                -d '{"model": "'"$model"'", "messages":[{"role": "user", "content": "'"$question"'"}]}' \
                https://api.openai.com/v1/chat/completions  | jq -r '.choices[0].message.content'
            )
        
        echo -e "\e[33m[$model][$conversation_file] Model:\e[0m $full_response"
        echo "$model|$conversation_file|Model: $full_response" >> ~/.chatgpt/$conversation_file

        remaining_tokens=$(curl -sS "https://api.openai.com/v1/usage?date=$(date +%Y-%m-%d)" -H "Authorization: Bearer $api_token" -H "Openai-Organization: $organization_id")
        echo "Remaining Tokens: $remaining_tokens"
    done
}

# Main function
main() {
    mkdir -p ~/.chatgpt
    
    if [[ $# -lt 1 ]]; then
        echo "Usage: ./chatgpt.sh <command> [parameters]"
        echo "Available commands: list-models, chat"
        exit 1
    fi
    
    command=$1
    shift

    case $command in
        list-models)
            echo -e "\e[31mlisting models using api token $1, model $2. Storing conversation in $3\e[0m"
            list_models "$1"
            ;;
        chat)
            echo -e "\e[31mStart of the conversation using api token $1, model $2. Storing conversation in $3\e[0m"
            chat $1 $2 $3
            ;;
        *)
            echo "Invalid command. Available commands: list-models, chat"
            exit 1
            ;;
    esac
}

# Run main function with arguments passed to the script
main $@
