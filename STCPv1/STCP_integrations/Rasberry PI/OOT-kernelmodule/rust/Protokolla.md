# Protokolla

## Todella nopee silmäys protokollaan

    1.  C => S (pubkey)
        Serveri laskee jaetun avaimen ja menee AES moodiin

    2.  S => C (pubkey)
        Clienti laskee jaetun avaimen ja menee AES moodiin

    3. AES moodi
        Jos jompi kumpi ei saa parsittua dataa => Yhteys rikki/epäluotettava => REFUSE & alasajo
        
