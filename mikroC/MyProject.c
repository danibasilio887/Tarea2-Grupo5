/* * Juego: Whac-A-Mole (con Niveles)
 * PIC16F887 (Cerebro)
 * MikroC for PIC
 */

// Variables del juego
unsigned short mole = 0;
unsigned short buttonPressed = 0;
unsigned int timer = 0;
unsigned int score = 0;
unsigned short level = 1;

// Variables de Dificultad
unsigned int base_timeout = 2000; // Nivel 1 (Principiante) = 2 segundos
unsigned int current_timeout = 2000; // El tiempo para esta ronda
unsigned int timeout_reduction = 30; // Reducción por acierto (ms)

// Constante para subir de nivel
const unsigned short LEVEL_UP_TARGET = 10;

// --- Funciones de Sonido (Completas) ---
void sol(int duration) { Sound_Play(392.00, duration); }
void la_sharp(int duration) { Sound_Play(466.16, duration); }
void do_prime(int duration) { Sound_Play(523.25, duration); }
void re_prime(int duration) { Sound_Play(587.33, duration); }
void re_sharp_prime(int duration) { Sound_Play(622.25, duration); }
void fa_prime(int duration) { Sound_Play(698.46, duration); }
void sol_prime(int duration) { Sound_Play(783.99, duration); }
void la_prime(int duration) { Sound_Play(880.00, duration); }

// Melodía de Victoria (Level Up)
void Melody_Win() {
    int d_short = 150;
    int d_long = 300;
    sol(d_short);
    do_prime(d_short);
    fa_prime(d_short);
    la_prime(d_long);
    sol_prime(d_long * 2);
    // (Puedes poner tu melodía larga aquí)
}

// Sonidos de Acierto y Fallo
void ToneA() { Sound_Play(880, 50); }
void ToneC() { Sound_Play(1046, 50); }
void ToneE() { Sound_Play(1318, 50); }
void Melody_Lose() {
    unsigned short i;
    for (i = 5; i > 0; i--) { // Melodía de derrota más corta
        ToneE(); ToneC(); ToneA();
    }
}
// --- FIN Funciones de Sonido ---


// Función MEJORADA de botones (Antirebote + Espera de Liberación)
unsigned short check_buttons() {
    unsigned short debounce_delay = 20;
    unsigned short active_state = 1; // 1 si VCC, 0 si GND

    if (Button(&PORTA, 0, debounce_delay, active_state)) {
        while(RA0_bit == active_state); return 1;
    }
    if (Button(&PORTA, 1, debounce_delay, active_state)) {
        while(RA1_bit == active_state); return 2;
    }
    if (Button(&PORTB, 0, debounce_delay, active_state)) {
        while(RB0_bit == active_state); return 3;
    }
    if (Button(&PORTB, 1, debounce_delay, active_state)) {
        while(RB1_bit == active_state); return 4;
    }
    if (Button(&PORTB, 2, debounce_delay, active_state)) {
        while(RB2_bit == active_state); return 5;
    }
    if (Button(&PORTB, 3, debounce_delay, active_state)) {
        while(RB3_bit == active_state); return 6;
    }
    if (Button(&PORTB, 4, debounce_delay, active_state)) {
        while(RB4_bit == active_state); return 7;
    }
    if (Button(&PORTB, 5, debounce_delay, active_state)) {
        while(RB5_bit == active_state); return 8;
    }
    if (Button(&PORTB, 6, debounce_delay, active_state)) {
        while(RB6_bit == active_state); return 9;
    }
    return 0;
}


void main() {
  // Configuraciones iniciales
  ANSEL  = 0; ANSELH = 0; C1ON_bit = 0; C2ON_bit = 0;
  TRISB  = 0xFF; TRISA  = 0xFF; TRISC  = 0x00; TRISD  = 0x00;
  Sound_Init(&PORTC, 3);
  srand(1);

  // --- SECUENCIA DE ARRANQUE ESTABLE ---
  // Estado inicial: (ESTADO=LOW, DATA=0) -> Mostrar "?"
  // El ATmega está mostrando "EMPECEMOS" por su cuenta.
  PORTD = 0x00;
  // Esperamos ~3 segundos a que el ATmega termine su scroll.
  Delay_ms(3000);

  // --- BUCLE DEL JUEGO ---
  while (1) {
    // 1. GENERAR EL "TOPO"
    mole = 1 + rand() % 9;
    buttonPressed = 0;
    timer = 0;
    current_timeout = base_timeout; // Asignar tiempo base del nivel

    // 2. MOSTRAR EL "TOPO"
    // (ESTADO=HIGH, DATA=mole)
    PORTD = (mole << 1) | (1 << 5);

    // Tocar sonido correspondiente al "topo"
    switch(mole) {
        case 1: sol(50); break;
        case 2: la_sharp(50); break;
        case 3: do_prime(50); break;
        case 4: re_prime(50); break;
        case 5: re_sharp_prime(50); break;
        case 6: fa_prime(50); break;
        case 7: sol_prime(50); break;
        case 8: la_prime(50); break;
        case 9: ToneE(); break;
    }

    // 3. ESPERAR ACIERTO O TIMEOUT
    while (timer < current_timeout) {
        buttonPressed = check_buttons();
        if (buttonPressed != 0) {
            // Aumentar dificultad para el *próximo* acierto
            if (current_timeout > 400) { // Poner un límite
                 current_timeout = current_timeout - timeout_reduction;
            }
            break;
        }
        Delay_ms(1);
        timer++;
    }

    // 4. VERIFICAR EL RESULTADO
    if (buttonPressed == mole) {
        // --- ACIERTO ---
        score++; // Incrementar puntuación

        ToneA(); ToneC(); ToneE(); // Sonido de acierto

        // --- Chequear Level Up ---
        if (score > 0 && (score % LEVEL_UP_TARGET == 0)) {
            level++;

            // 1. Enviar comando de animación al ATmega
            // (ESTADO=LOW, DATA=3)
            PORTD = (3 << 1) & ~(1 << 5);

            // 2. Tocar melodía de victoria
            Melody_Win(); // Tocar mientras el ATmega se anima

            // 3. Aumentar dificultad permanentemente para el prox. nivel
            if (base_timeout > 800) {
                base_timeout = base_timeout - 300; // Reduce el tiempo base
            }
            if (timeout_reduction < 100) {
                timeout_reduction = timeout_reduction + 15; // Aumenta la reducción
            }

        } else {
            // Si no es level up, solo mostrar 'O' de acierto
            // (ESTADO=LOW, DATA=2)
            PORTD = (2 << 1) & ~(1 << 5);
            Delay_ms(500); // Pausa para ver el círculo
        }

        // Mostrar '?' (en espera)
        // (ESTADO=LOW, DATA=0)
        PORTD = 0x00;
        Delay_ms(200);

    } else {
        // --- FALLO (Botón incorrecto O se acabó el tiempo) ---

        Melody_Lose(); // Tocar melodía de derrota

        // Mostrar 'X' (PERDER)
        // (ESTADO=LOW, DATA=1)
        PORTD = (1 << 1) & ~(1 << 5);
        Delay_ms(3000); // Pausa larga para que el jugador vea la 'X'

        // --- Reiniciar toda la dificultad ---
        score = 0;
        level = 1;
        base_timeout = 2000; // Volver a Principiante
        timeout_reduction = 30; // Volver a reducción fácil

        // Mostrar '?' (en espera)
        // (ESTADO=LOW, DATA=0)
        PORTD = 0x00;
        Delay_ms(1000);
    }

  } // Fin del while(1)
}
[PROJECTS]
Count=0
[ActiveProject]
Index=0
