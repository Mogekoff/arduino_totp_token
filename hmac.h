String HMAC(String msg, String K,String (*H)(String text), const int b){
    String buf; 
    Serial.println(msg);
    if(K.length() > b)              //Если ключ больше размера блока, укорачиваем по хешу
        K = H(K);
    while(K.length() < b)           //Если меньше, дополняем нулями
        K+=('\0');
    
    auto k = K;                     //Копия ключа для будущих вычислений
    for(auto i = 0; i < b; i++)
        k[i] = char(k[i] ^ 0x36);
    
    buf=H(k+msg);
    //Serial.println(buf);
    for (auto i = 0; i < b; i++)
        K[i] ^= 0x5c;
    
    //Конв. строку hex символов в строку байт
    for (auto i = 0; i < buf.length(); i++) {
        buf[i] = (uint8_t)strtol(buf.substring(i, 2).c_str(), 0, 16);
        buf.remove(i + 1,1);
    }

    return H(K + buf);
}
