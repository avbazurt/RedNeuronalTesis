#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define DEBUG_PRINTLN(texto)          \
  Serial.print("[");          \
  Serial.print(__FILENAME__); \
  Serial.print("]:");         \
  Serial.println(texto);

#define DEBUG_PRINT(texto)     \
  Serial.print("[");          \
  Serial.print(__FILENAME__); \
  Serial.print("]:");          \
  Serial.print(texto);