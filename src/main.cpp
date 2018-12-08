#ifndef UNIT_TEST

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WString.h>
#include <ESP8266HTTPClient.h>
#include <Streams/StringStream.h>
#include <JsonStreamTokenizer.h>

void setup() {
  Serial.begin(115200);
  Serial.println();

  //String json = "{\"name\":\"Michael\",\"lights\":[\"11\",\"2\"],\"sensors\":[],\"type\":\"Room\",\"state\":{\"all_on\":true,\"any_on\":true},\"recycle\":false,\"class\":\"Office\",\"action\":{\"on\":true,\"bri\":254,\"hue\":8597,\"sat\":121,\"effect\":\"none\",\"xy\":[0.4452,0.4068],\"ct\":343,\"alert\":\"none\",\"colormode\":\"xy\"}}";
  String json2 = "   {\"a\":\"What is this BS?\",\"b\":true}";
  //String test = "[1,2,3,4,]";

  JsonStreamTokenizer* tokenizer = new JsonStreamTokenizer();
  tokenizer->parse(new StringStream(json2));

  while(tokenizer->hasNext()) {
      JsonStreamTokenizer::Token t = tokenizer->next();
      Serial.print(t.val);
      if(t.type == JsonStreamTokenizer::TokenType::PARSE_ERROR) break;
  }
}

void loop() {
  delay(10000);
}

#endif