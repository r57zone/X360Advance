float ypr[3];

void setup() {
   Serial.begin(115200);
}

void Read() {
   //MPU 6050 or etc
   //Read data from rotation sensors board and convert to degrees
}

void loop() {
    Read();
    Serial.write((byte*) ypr, 12);
}
