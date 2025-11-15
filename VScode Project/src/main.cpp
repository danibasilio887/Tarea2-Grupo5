#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 8000000

// Pines de comunicación
#define RD5_PIN    PC5
#define START_PIC  PC0

// --- Arreglos de imágenes (Constantes) ---
unsigned char signo[8] = {0x0, 0x04, 0x02, 0x01, 0xB1, 0x0A, 0x04, 0x0}; // '?'
unsigned char PERDER[8] = {0x81, 0xC3, 0x66, 0x18, 0x18, 0x66, 0xC3, 0x81}; // 'X'
unsigned char CIRCULO[8] = {0x00, 0x3C, 0x42, 0x81, 0x81, 0x42, 0x3C, 0x00}; // 'O'
unsigned char EMPECEMOS[80] = { // Mensaje "EMPECEMOS"
    0x0, 0x7E, 0x7E, 0x5A, 0x5A, 0x5A, 0x5A, 0x0, // E
    0x0, 0x7E, 0x04, 0x08, 0x08, 0x04, 0x7E, 0x0, // M
    0x0, 0x7E, 0x7E, 0x12, 0x12, 0x1E, 0x1E, 0x0, // P
    0x0, 0x7E, 0x7E, 0x5A, 0x5A, 0x5A, 0x5A, 0x0, // E
    0x0, 0x7E, 0x7E, 0x42, 0x42, 0x42, 0x42, 0x0, // C
    0x0, 0x7E, 0x7E, 0x5A, 0x5A, 0x5A, 0x5A, 0x0, // E
    0x0, 0x7E, 0x04, 0x08, 0x08, 0x04, 0x7E, 0x0, // M
    0x0, 0x7E, 0x7E, 0x66, 0x66, 0x7E, 0x7E, 0x0, // O
    0x0, 0x4E, 0x4E, 0x5A, 0x5A, 0x72, 0x72, 0x0, // s
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0       // ESPACIO
};

// Arreglo para fijar filas
unsigned char PORT[8] = {1, 2, 4, 8, 16, 32, 64, 128};

// --- Arreglos para numéros (0-9) ---
unsigned char cero[8] = {0x00, 0x7E, 0xFF, 0x81, 0x81, 0xFF, 0x7E, 0x00}; 
unsigned char uno[8] = {0x0, 0x0, 0x81, 0xFF, 0xFF, 0x80, 0x0, 0x0};
unsigned char dos[8] = {0x0, 0xC2, 0xE3, 0xB1, 0x99, 0x8F, 0x86, 0x0};
unsigned char tres[8] = {0x0, 0xC3, 0x99, 0x99, 0x99, 0xFF, 0xFF, 0x0};
unsigned char cuatro[8] = {0x0, 0x1F, 0x1F, 0x18, 0x18, 0xFF, 0xFF, 0x0};
unsigned char cinco[8] = {0x0, 0x8F, 0x8F, 0x89, 0x89, 0xF9, 0xF9, 0x0};
unsigned char seis[8] = {0x0, 0xFF, 0xFF, 0x91, 0x91, 0xF1, 0xF3, 0x0};
unsigned char siete[8] = {0x0, 0x03, 0xE1, 0xF1, 0x19, 0x0D, 0x07, 0x0};
unsigned char ocho[8] = {0x0, 0xF7, 0xFF, 0x99, 0x99, 0xFF, 0xF7, 0x0};
unsigned char nueve[8] = {0x0, 0x0F, 0x0F, 0x09, 0x09, 0xFF, 0xFF, 0x0};

// --- Estado de la pantalla para animaciones ---
unsigned char displayState[8];

// Función para mostrar imágenes CONSTANTES (como 'X', 'O', '1', etc.)
void mostrarMatriz(int side, const unsigned char* imagen) {
    if (side > 8) { // Para scroll
        for (int i = 0; i < side - 8; i++) {
            for (int k = 0; k < 50; k++) {
                for (int j = 0; j < 8; j++) {
                    PORTD = PORT[j];
                    PORTB = ~imagen[i + j];
                    _delay_ms(0.05);
                }
            }
        }
    } 
    if (side == 8) { // Para imagen estática
        for (int k = 0; k < 50; k++) { // Muestra por 20ms
            for (int j = 0; j < 8; j++) {
                PORTD = PORT[j];
                PORTB = ~imagen[j];
                _delay_ms(0.05);
            }
        }
    }
}

// Función para mostrar el estado DINÁMICO (para la animación)
void mostrarDisplay() {
    // Esta función renderiza el array 'displayState' una vez (aprox 20ms)
    for (int k = 0; k < 50; k++) { 
        for (int j = 0; j < 8; j++) {
            PORTD = PORT[j];
            PORTB = ~displayState[j]; // Usa el array 'displayState'
            _delay_ms(0.05);
        }
    }
}

// Función de animación de subida de nivel
void animacionLevelUp() {
    // 1. Pantalla toda encendida
    for(int i=0; i<8; i++) { 
        displayState[i] = 0xFF; // 0xFF = 11111111 (todo encendido)
    }
    
    // Mostrarla por ~500ms (25 * 20ms = 500ms)
    for(int i=0; i<25; i++) { 
        mostrarDisplay(); 
    }

    // 2. Apagado diagonal (15 pasos, desde (0,0) hasta (7,7))
    for (int diag = 0; diag <= 14; diag++) {
        for (int r = 0; r < 8; r++) { // row
            for (int c = 0; c < 8; c++) { // col
                if (r + c == diag) {
                    // Apaga el bit (c) en la fila (r)
                    displayState[r] &= ~(1 << c); 
                }
            }
        }
        
        // Mostrar este frame por ~100ms (5 * 20ms = 100ms)
        for(int i=0; i<5; i++) { 
            mostrarDisplay(); 
        }
    }
}

// Función para leer los pines de datos del PIC
unsigned char binarioADecimal() {
    return ((PINC & 0x1E) >> 1);  // Lee los 4 bits de datos
}

int main(void) {
    // Configuración de pines
    DDRB = 0xFF;  // Salida (columnas)
    DDRD = 0xFF;  // Salida (filas)
    DDRC = 0x01;  // Entrada (PC1-PC5), Salida (PC0)
    PORTC = 0x00; // Desactivar pull-ups

    // --- SECUENCIA DE ARRANQUE ESTABLE ---
    // 1. Mostrar "EMPECEMOS"
    mostrarMatriz(80, EMPECEMOS);
    // 2. Mostrar "?" (esperando que el PIC inicie)
    mostrarMatriz(8, signo);
    
    PORTC |= (1 << START_PIC); // (Opcional, no hace daño)

    // --- BUCLE PRINCIPAL ---
    while (1) {
        unsigned char data = binarioADecimal();

        if (PINC & (1 << RD5_PIN)) {
            // ESTADO = HIGH -> Juego activo, mostrar el "topo" (1-9)
            switch (data) {
                case 1: mostrarMatriz(8, uno); break;
                case 2: mostrarMatriz(8, dos); break;
                case 3: mostrarMatriz(8, tres); break;
                case 4: mostrarMatriz(8, cuatro); break;
                case 5: mostrarMatriz(8, cinco); break;
                case 6: mostrarMatriz(8, seis); break;
                case 7: mostrarMatriz(8, siete); break;
                case 8: mostrarMatriz(8, ocho); break;
                case 9: mostrarMatriz(8, nueve); break;
                default: mostrarMatriz(8, signo); 
            }
        } else {
            // ESTADO = LOW -> Mostrar resultado o animación
            switch (data) {
                // Resultados
                case 0: mostrarMatriz(8, signo); break;   // '?'
                case 1: mostrarMatriz(8, PERDER); break;  // 'X'
                case 2: mostrarMatriz(8, CIRCULO); break; // 'O'
                
                // Comando de Animación (Level Up)
                case 3: 
                    animacionLevelUp(); // Ejecutar animación
                    break;
                
                default:
                    mostrarMatriz(8, signo);
                    break;
            }
        }
    }
    return 0;
}