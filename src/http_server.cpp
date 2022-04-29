#include <esp_http_server.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_vfs.h"
#include "wifi_client.h"
#include "http_server.h"

static const char *TAG = "http server";

// Scratch buffer size
#define SCRATCH_BUFSIZE  8192

struct file_server_data {
    /* Base path of file storage */
    char base_path[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
} server_data;

static esp_err_t get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    const char *filename = "/data/index.html";
    FILE *fd = fopen(filename, "r");
    struct stat file_stat;

    if (stat(filename, &file_stat) == -1) {
        ESP_LOGE(TAG, "Failed to stat file : %s", filename);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
               return ESP_FAIL;
           }
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t startWebserver(const char* base_path)
{
    initWiFi();
    strlcpy(server_data.base_path, base_path, sizeof(server_data.base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

    esp_err_t startCode = httpd_start(&server, &config);
    if (startCode == ESP_OK) {
        // register handlers
        httpd_uri_t uri_get = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = get_handler,
            .user_ctx = &server_data,
        };
        httpd_register_uri_handler(server, &uri_get);
    } else {
        ESP_LOGE(TAG, "Failed to start HTTP server: %d", startCode);
    }
    // If server failed to start, handle will be NULL
    return startCode;
}