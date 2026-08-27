extern "C" {
  #include "button_fsm.h"
}

static const uint8_t  BUTTON_PIN   = 2;
static const uint32_t DEBOUNCE_MS  = 25;

static button_fsm_t   fsm;
static button_level_t stable_level = BTN_RELEASED;
static button_level_t last_raw     = BTN_RELEASED;
static uint32_t       last_change  = 0;

static button_level_t read_button_debounced() {
  button_level_t raw = (digitalRead(BUTTON_PIN) == LOW) ? BTN_PRESSED : BTN_RELEASED;
  uint32_t now = millis();
  if (raw != last_raw) {
    last_raw = raw;
    last_change = now;             
  } else if ((now - last_change) >= DEBOUNCE_MS) {
    stable_level = raw;              
  }
  return stable_level;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  button_fsm_init(&fsm, 10000);    
}

void loop() {
  button_level_t lvl = read_button_debounced();
  static button_state_t prev = BTN_STATE_OFF;
  button_state_t st = button_fsm_update(&fsm, lvl, millis());

  if (st != prev) {
    Serial.print("Estado -> ");
    Serial.println(button_state_name(st));
    if (prev == BTN_STATE_ON && st == BTN_STATE_PROTECTED) {
      Serial.print("Duracao do pressionamento = ");
      Serial.print(button_fsm_last_press_duration(&fsm));
      Serial.println(" ms");
    }
    prev = st;
  }
}
