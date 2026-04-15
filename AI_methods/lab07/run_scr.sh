#!/bin/bash

set -e
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Запуск демонстрации VLM модели Ministral-3${NC}"
echo -e "${GREEN}========================================${NC}"

# Проверка установки Ollama
if ! command -v ollama &> /dev/null; then
    echo -e "${RED}Ошибка: Ollama не установлен!${NC}"
    echo "Установите его командой: curl -fsSL https://ollama.com/install.sh | sh"
    exit 1
fi
IMAGE_PATH="${1:-./test_image.jpg}"
if [ ! -f "$IMAGE_PATH" ]; then
    echo -e "${YELLOW}Предупреждение: Файл $IMAGE_PATH не найден!${NC}"
    echo "Создаю тестовое изображение через ImageMagick..."
    
    if command -v convert &> /dev/null; then
        # Создаём простое тестовое изображение с текстом
        convert -size 800x600 xc:lightblue \
            -font Helvetica -pointsize 40 \
            -fill darkblue -draw "text 100,200 'Hello VLM!'" \
            -draw "text 100,300 'Это тестовое изображение'" \
            -draw "rectangle 50,50 750,550" \
            "$IMAGE_PATH"
        echo -e "${GREEN}✓ Создано тестовое изображение: $IMAGE_PATH${NC}"
    else
        echo -e "${RED}Ошибка: ImageMagick не установлен для создания тестового изображения${NC}"
        echo "Установите: sudo apt install imagemagick"
        echo "Или укажите путь к существующему изображению: $0 /path/to/image.jpg"
        exit 1
    fi
fi

echo -e "${GREEN}✓ Использую изображение: $IMAGE_PATH${NC}"
echo -e "${YELLOW}Проверка наличия модели...${NC}"
if ! ollama list | grep -q "frob/ministral-3:8b-thinking-fp16"; then
    echo -e "${YELLOW}Модель не найдена. Загружаю...${NC}"
    ollama pull frob/ministral-3:8b-thinking-fp16
    echo -e "${GREEN}✓ Модель загружена${NC}"
else
    echo -e "${GREEN}✓ Модель уже существует${NC}"
fi

# Функция для отправки запроса к модели
ask_model() {
    local question="$1"
    local image="$2"
    
    echo -e "\n${YELLOW}Вопрос: ${question}${NC}"
    echo -e "${GREEN}Ответ модели:${NC}"
    echo "----------------------------------------"
    
    # Отправляем запрос с изображением
    echo "$question $image" | ollama run frob/ministral-3:8b-thinking-fp16 2>/dev/null
    
    echo "----------------------------------------"
    echo ""
}

# Вопрос 1: Общее описание
ask_model "Опиши, что ты видишь на этом изображении. Будь краток (1-2 предложения):" "$IMAGE_PATH"

# Вопрос 2: Конкретный вопрос
ask_model "Какие основные цвета присутствуют на этом изображении? Перечисли их:" "$IMAGE_PATH"

# Вопрос 3: Обнаружение объектов
ask_model "Есть ли на изображении геометрические фигуры? Если да, то какие?" "$IMAGE_PATH"

# Вопрос 4: Контекстный вопрос
ask_model "Какое настроение или атмосферу создает это изображение?" "$IMAGE_PATH"

# Сохраняем результаты в файл
LOG_FILE="./vlm_demo_results_$(date +%Y%m%d_%H%M%S).txt"
echo -e "${GREEN}Сохранение результатов в $LOG_FILE${NC}"
{
    echo "=== Демонстрация VLM Ministral-3 ==="
    echo "Дата: $(date)"
    echo "Изображение: $IMAGE_PATH"
    echo "====================================="
    echo ""
    echo "Вопрос 1: Опиши, что ты видишь на этом изображении"
    echo "Ответ: $(echo "$question" | ollama run frob/ministral-3:8b-thinking-fp16 2>/dev/null)"
} > "$LOG_FILE"

echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}Демонстрация завершена!${NC}"
echo -e "${GREEN}========================================${NC}"