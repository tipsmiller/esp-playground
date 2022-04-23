#include <esp_http_server.h>
#include "esp_log.h"
#include "http_server.h"

static const char *TAG = "http server";

esp_err_t get_handler(httpd_req_t *req)
{
    const char resp[] = "Hi there :)";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

    esp_err_t startCode = httpd_start(&server, &config);
    if (startCode == ESP_OK) {
        // register handlers
        httpd_register_uri_handler(server, &uri_get);
    } else {
        ESP_LOGE(TAG, "Failed to start HTTP server: %d", startCode);
    }
    // If server failed to start, handle will be NULL
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}