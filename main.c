#include "./mongoose/mongoose.h"
#include "./input/input.h"
#include "./constants/constants.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

enum {
    ERR_OK = 0,
    ERR_FILE_NOT_FOUND = 1
};

// Функция для разделения строки на элементы списка
static void split_string(const char *input, char ***items, int *count) {
    char *copy = strdup(input);
    char *token = strtok(copy, ",");
    *count = 0;
    *items = NULL;
    
    while (token != NULL) {
        // Удаляем пробелы в начале и конце
        while (*token == ' ') token++;
        int len = strlen(token);
        while (len > 0 && token[len-1] == ' ') len--;
        token[len] = '\0';
        
        if (len > 0) {
            *items = realloc(*items, (*count + 1) * sizeof(char*));
            (*items)[*count] = strdup(token);
            (*count)++;
        }
        token = strtok(NULL, ",");
    }
    free(copy);
}

// Функция для генерации HTML-списка
static char *generate_list_html(const char *items_str, const char *list_type) {
    char **items = NULL;
    int count = 0;
    split_string(items_str, &items, &count);
    
    // Вычисляем необходимый размер буфера
    size_t buffer_size = 1024; // Начальный размер
    for (int i = 0; i < count; i++) {
        buffer_size += strlen(items[i]) + 20; // +20 для тегов и прочего
    }
    
    char *html = malloc(buffer_size);
    if (!html) return NULL;
    
    // Начинаем формировать HTML
    strcpy(html, "<!DOCTYPE html>\n<html lang=\"ru\">\n<head>\n");
    strcat(html, "<meta charset=\"UTF-8\">\n<title>Результат</title>\n");
    strcat(html, "<link rel=\"stylesheet\" href=\"styles.css\">\n</head>\n<body>\n");
    strcat(html, "<div class=\"list-container\">\n");
    
    // Добавляем заголовок
    strcat(html, "<h2>Ваш список:</h2>\n");
    
    // Добавляем список
    if (strcmp(list_type, "numbered") == 0) {
        strcat(html, "<ol>\n");
    } else {
        strcat(html, "<ul>\n");
    }
    
    for (int i = 0; i < count; i++) {
        strcat(html, "<li>");
        strcat(html, items[i]);
        strcat(html, "</li>\n");
        free(items[i]);
    }
    
    if (strcmp(list_type, "numbered") == 0) {
        strcat(html, "</ol>\n");
    } else {
        strcat(html, "</ul>\n");
    }
    
    // Добавляем кнопку возврата
    strcat(html, "<div class=\"back-link\"><a href=\"/\">Создать новый список</a></div>\n");
    strcat(html, "</div>\n</body>\n</html>");
    
    free(items);
    return html;
}

static int process_request(struct mg_connection *c, struct mg_http_message *hm) {
    int status_code = 500;
    const char *ctype = "";
    char *response = NULL;
    int error_code = ERR_FILE_NOT_FOUND;

    // Обработка POST-запроса для генерации списка
    if (!mg_strcmp(hm->uri, mg_str("/generate-list")) && 
        !mg_strcasecmp(hm->method, mg_str("POST"))) {
        char items_str[1024], list_type[20];
        mg_http_get_var(&hm->body, "items", items_str, sizeof(items_str));
        mg_http_get_var(&hm->body, "list_type", list_type, sizeof(list_type));
        
        // Генерируем HTML со списком
        response = generate_list_html(items_str, list_type);
        
        if (response) {
            status_code = 200;
            ctype = CONTENT_TYPE_HTML;
            error_code = ERR_OK;
        }
    }
    // Запрос CSS-файла
    else if (!mg_strcmp(hm->uri, mg_str("/styles.css"))) {
        response = read_file(PATH_CSS_STYLES);
        if (response) {
            status_code = 200;
            ctype = CONTENT_TYPE_CSS;
            error_code = ERR_OK;
        }
    }
    // Все остальные запросы - главная страница
    else {
        response = read_file(PATH_INDEX_HTML);
        if (response) {
            status_code = 200;
            ctype = CONTENT_TYPE_HTML;
            error_code = ERR_OK;
        }
    }

    if (error_code == ERR_OK) {
        mg_http_reply(c, status_code, ctype, "%s", response);
    } else {
        mg_http_reply(c, 500, "", "");
    }
    
    free(response);
    return error_code;
}

static void main_fun(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        process_request(c, hm);
    }
}

int main(void) {
    const char *server_address = "http://localhost:8081";
    struct mg_mgr mgr;
    
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, server_address, main_fun, NULL);
    
    printf("Сервер запущен на %s\n", server_address);
    
    for (;;) mg_mgr_poll(&mgr, 1000);
    
    mg_mgr_free(&mgr);
    return 0;
}
