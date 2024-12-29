#include "driver/i2s.h"
#include "esp_adc/adc_continuous.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#define SAMPLE_RATE 50000 // 50 kSPS por canal
#define NUM_CHANNELS 3    // Número de canais ADC
#define BUF_LEN 1024      // Tamanho do buffer
#define SAMPLE_TIME 1     // Tempo de amostragem em segundos
#define TAG "I2S_ADC"

// Configuração dos canais ADC
static const adc1_channel_t adc_channels[NUM_CHANNELS] = {
    ADC1_CHANNEL_0, // GPIO36
    ADC1_CHANNEL_3, // GPIO39
    ADC1_CHANNEL_6  // GPIO34
};

// Função para inicializar o I2S no modo ADC
void i2s_adc_init() {
  // Configuração do I2S
  i2s_config_t i2s_conf = {
      .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
      .sample_rate = SAMPLE_RATE * NUM_CHANNELS,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = 0,
      .dma_buf_count = 2,
      .dma_buf_len = BUF_LEN,
      .use_apll = false,
  };

  ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_conf, 0, NULL));
  ESP_ERROR_CHECK(i2s_set_adc_mode(ADC_UNIT_1, adc_channels[0]));
  ESP_ERROR_CHECK(i2s_adc_enable(I2S_NUM_0));

  // Configuração dos canais ADC
  adc1_config_width(ADC_WIDTH_BIT_12); // Resolução de 12 bits
  for (int i = 0; i < NUM_CHANNELS; i++) {
    adc1_config_channel_atten(adc_channels[i], ADC_ATTEN_DB_12);
  }
}

// Função principal para leitura dos valores ADC
void i2s_adc_read_task(void *arg) {
  size_t bytes_read;
  uint16_t *buf = malloc(BUF_LEN * sizeof(uint16_t));
  uint32_t sums[NUM_CHANNELS] = {0};
  uint32_t sample_count = 0;

  if (!buf) {
    ESP_LOGE(TAG, "Falha ao alocar buffer");
    vTaskDelete(NULL);
  }

  while (1) {
    // Limpa somas e contadores
    memset(sums, 0, sizeof(sums));
    sample_count = 0;

    // Leitura por SAMPLE_TIME segundos
    for (int t = 0; t < SAMPLE_RATE * SAMPLE_TIME / BUF_LEN; t++) {
      ESP_ERROR_CHECK(i2s_read(I2S_NUM_0, buf, BUF_LEN * sizeof(uint16_t), &bytes_read, portMAX_DELAY));

      // Processa os dados lidos
      for (size_t i = 0; i < bytes_read / 2; i++) {
        int channel = (i % NUM_CHANNELS); // Identifica o canal
        sums[channel] += buf[i] & 0x0FFF; // Coleta valor (12 bits úteis)
      }
      sample_count += (bytes_read / 2) / NUM_CHANNELS;
    }

    // Calcula e exibe médias
    for (int i = 0; i < NUM_CHANNELS; i++) {
      printf("Canal %d: Média = %lu\n", i, sums[i] / sample_count);
    }
    printf("-----------------------------\n");

    // Delay para próxima leitura
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  free(buf);
  vTaskDelete(NULL);
}

void app_main(void) {
  // Inicialização do ADC via I2S
  i2s_adc_init();

  // Cria a tarefa para leitura contínua
  xTaskCreate(i2s_adc_read_task, "i2s_adc_read_task", 4096, NULL, 5, NULL);
}
