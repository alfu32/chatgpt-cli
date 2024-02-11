#!/bin/bash

# Prompt user to enter their openai api key
read -p "Enter your openai API key: " YOUR_OPENAI_API_KEY

# List available models
echo "Available models:"
echo "1. gpt-3.5-turbo"
echo "2. gpt-3.5-turbo-davinci"
echo "3. gpt-3.5-turbo-curie"
echo "4. gpt-3.5-turbo-babbage"
echo "5. gpt-3.5-turbo-ada"

# Prompt user to choose a model
read -p "Enter the number of the model you'd like to use [1.gpt-3.5-turbo]: " model_number

case $model_number in
    1)
        model="gpt-3.5-turbo"
        ;;
    2)
        model="gpt-3.5-turbo-davinci"
        ;;
    3)
        model="gpt-3.5-turbo-curie"
        ;;
    4)
        model="gpt-3.5-turbo-babbage"
        ;;
    5)
        model="gpt-3.5-turbo-ada"
        ;;
    *)
        echo "using default : gpt-3.5-turbo"
        model="gpt-3.5-turbo"
        ;;
esac

# Start conversation
while true; do
echo -e "\e[32mYou: \e[0m"
    read -p "" user_input
    # Break loop if user types "exit"
    if [ "$user_input" == "exit" ]; then
        echo "Goodbye!"
        break
    fi
    # Call OpenAI API with the chosen model
    echo -e "\e[31mAI:\e[0m"
    response=$(curl -s -X POST -H "Content-Type: application/json" \
        -H "Authorization: Bearer $YOUR_OPENAI_API_KEY" \
        -d '{"model": "'"$model"'", "messages":[{"role": "user", "content": "'"$user_input"'"}]}' \
        https://api.openai.com/v1/chat/completions)
    # Extract and display AI response
    ai_response=$(echo "$response" | jq -r '.choices[0].message.content')
    // echo $response
    echo "$ai_response"
done
