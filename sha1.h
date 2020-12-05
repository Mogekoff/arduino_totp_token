struct buffer {
    uint8_t* array;
    int size = 0;
    void push_back(uint8_t elem) {
        auto* new_array = new uint8_t[size+1];
        for(auto i = 0; i<size; i++)
            new_array[i]=array[i];
        new_array[size]=elem;
        delete[] array;
        array=new_array;
        size++;
    }
};

//Константы
uint32_t A;
uint32_t B;
uint32_t C;
uint32_t D;
uint32_t E;

//Функция цикл. сдвига влево. x - аргумент, n - сдвиг
uint32_t shiftLeft(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
}

//Ф-я добавления длины сообщения
void addingLenght(buffer& buf, const uint64_t len) {
    for (auto i = 7; i >= 0; i--)
        buf.push_back(uint8_t((len >> (i * 8)) & 0xFF));
}

//Ф-я дополнения сообщения нулями
void addingNulls(buffer& buf) {
    do
        buf.push_back(0);
    while (buf.size % 64 != 56);
}

//Ф-я добавления единичного байта
void adding1byte(buffer& buf) {
    buf.push_back(128);   //128 = 1000 0000
}

//Ф-я конвертирования 512 битного блока сооб-я в 80 слов по 32 бита
uint32_t* conv512to80(buffer& buf, int offset) {
    uint32_t* words = new uint32_t[80];

    for (auto i = 0; i < 16; i++) {
        words[i] = 0;
        words[i] += uint32_t(buf.array[4 * i + offset]) << 24;
        words[i] += uint32_t(buf.array[4 * i + 1 + offset]) << 16;
        words[i] += uint32_t(buf.array[4 * i + 2 + offset]) << 8;
        words[i] += uint32_t(buf.array[4 * i + 3 + offset]);
    }
    for (auto i = 16; i < 80; i++)
        words[i] = shiftLeft(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);

    return words;
}

//Ф-я вычисления раундов
void calcBlocks(uint32_t* buf) {
    uint32_t i, k, f;
    uint32_t a = A, b = B, c = C, d = D, e = E;
    for (i = 0; i < 80; i++) {
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        }
        else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        }
        else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        }
        else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        uint32_t temp = shiftLeft(a, 5) + f + e + k + buf[i];
        e = d;
        d = c;
        c = shiftLeft(b, 30);
        b = a;
        a = temp;
    }
    A += a;
    B += b;
    C += c;
    D += d;
    E += e;
    delete[] buf;
}

String hex(uint32_t num){
  auto hex_string = String(num,HEX);
  while(hex_string.length()<8)
    hex_string='0'+hex_string;
  return hex_string;
  }
//Основная ф-я, вызывающая вспомогательные
String sha1(const String msg) {
    uint64_t len = msg.length();

    buffer buf;
    buf.array = new uint8_t[len];
    buf.size = len;

    for (auto i = 0; i < len; i++)
        buf.array[i] = msg[i];

    len *= 8; //Размер сообщения в битах

    //Добавляем единичный байт(бит)
    adding1byte(buf);
    //Выравнивание потока нулями
    addingNulls(buf);
    //Записываем длину сообщения размером 64 бит до выравнивания в блок 512 бит
    addingLenght(buf, len);
    //Инициализируем константы
    A = 0x67452301;
    B = 0xEFCDAB89;
    C = 0x98BADCFE;
    D = 0x10325476;
    E = 0xC3D2E1F0;

    //Преобразуем блок 512 бит в массив из 80 слов по 32 бита и начинаем вычислять раунды
    for (auto offset = 0; offset < buf.size; offset += 64)
        calcBlocks(conv512to80(buf, offset));
    delete[] buf.array;
    //Возвращаем конк. строк 16-чных представлений чисел
    return hex(A)+hex(B)+hex(C)+hex(D)+hex(E);
}
