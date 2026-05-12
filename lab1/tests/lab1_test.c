#include <stdio.h>
#include <stdlib.h>
#include <math.h>      // Потребует libm
#include <pthread.h>   // Потребует libpthread
#include <unistd.h>

// Функция для выполнения в потоке
void* calculate_sqrt(void* arg) {
    double* num = (double*)arg;
    printf("[Thread] Вычисляем корень из %.2f...\n", *num);
    sleep(1);
    double result = sqrt(*num);
    printf("[Thread] Результат: %.4f\n", result);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Использование: %s <число>\n", argv[0]);
        return 1;
    }

    double input = atof(argv[1]);
    pthread_t thread;

    printf("[Main] Запуск потока для математических вычислений...\n");

    // Создание потока
    if (pthread_create(&thread, NULL, calculate_sqrt, &input) != 0) {
        perror("Ошибка создания потока");
        return 1;
    }

    // Ожидание завершения потока
    pthread_join(thread, NULL);

    printf("[Main] Программа завершена.\n");
    return 0;
}