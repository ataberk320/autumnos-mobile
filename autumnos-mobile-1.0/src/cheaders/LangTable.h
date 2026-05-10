
#ifndef DICT_H
#define DICT_H

static const char* lang_tr[] = {
	[STR_PHONE] = "Telefon", [STR_MSG] = "Mesajlar", [STR_CAMERA] = "Kamera",
	[STR_CONTACTS] = "Kişiler", [STR_MUSIC] = "Müzik", [STR_GALLERY] = "Galeri",
	[STR_BROWSER] = "S-Browser", [STR_SETTINGS] = "Ayarlar",  [STR_CALNOT] = "Takvim", [STR_MAIL] = "Autumn Mail",
	[STR_STORE] = "Autumn Store", [STR_CALCULATE] = "Hesaplama", [STR_GIT] = "GitBrowser", [STR_FILES] = "Dosyalar", [STR_WIKI] = "Wikipedia",
	

	[STR_JAN] = "Ocak", [STR_FEB] = "Şubat", [STR_MAR] = "Mart",
	[STR_APR] = "Nisan", [STR_MAY] = "Mayıs", [STR_JUN] = "Haziran",
	[STR_JUL] = "Temmuz", [STR_AUG] = "Ağustos", [STR_SEP] = "Eylül",
	[STR_OCT] = "Ekim", [STR_NOV] = "Kasım", [STR_DEC] = "Aralık",

	[STR_SUN] = "Pazar", [STR_MON] = "Pazartesi", [STR_TUE] = "Salı",
	[STR_WED] = "Çarşamba", [STR_THU] = "Perşembe", [STR_FRI] = "Cuma",
	[STR_SAT] = "Cumartesi"

	[STR_SUNNY] = "Güneşli", [STR_RAINY] = "Yağmurlu", [STR_SNOWY] = "Karlı",
	[STR_PARTLY] = "Parçalı bulutlu", [STR_CLOUDY] = "Bulutlu", [STR_UNKNOWN] = "Bağlantı Yok",
	
	[STR_UNK_CALL_ID] = "Bilinmiyor", [STR_RES_CALL_ID] = "Kısıtlanmış", [STR_PRV_CALL_ID] = "Gizli numara", [STR_UNAV_CALL_ID] = "Kullanılamıyor",
	[STR_CALL_DCL] = "Reddetmek için güç tuşuna basın", [STR_CALL_ANS] = "Yanıtlamak için yukarı kaydırın", [STR_INC_CALL] = "GELEN ARAMA"
};

static const char* lang_en[] = {
        [STR_PHONE] = "Phone", [STR_MSG] = "Messages", [STR_CAMERA] = "Camera",
        [STR_CONTACTS] = "Contacts", [STR_MUSIC] = "Music", [STR_GALLERY] = "Gallery",
        [STR_BROWSER] = "S-Browser", [STR_SETTINGS] = "Settings",  [STR_CALNOT] = "Calendar", [STR_MAIL] = "Autumn Mail",
        [STR_STORE] = "Autumn Store", [STR_CALCULATE] = "Calculate", [STR_GIT] = "GitBrowser", [STR_FILES] = "Files", [STR_WIKI] = "Wikipedia",


        [STR_JAN] = "January", [STR_FEB] = "February", [STR_MAR] = "March",
        [STR_APR] = "April", [STR_MAY] = "May", [STR_JUN] = "June",
        [STR_JUL] = "July", [STR_AUG] = "August", [STR_SEP] = "September",
        [STR_OCT] = "October", [STR_NOV] = "November", [STR_DEC] = "December",

        [STR_SUN] = "Sunday", [STR_MON] = "Monday", [STR_TUE] = "Tuesday",
        [STR_WED] = "Wednesday", [STR_THU] = "Thursday", [STR_FRI] = "Friday",
        [STR_SAT] = "Saturday"

        [STR_SUNNY] = "Sunny", [STR_RAINY] = "Rainy", [STR_SNOWY] = "Snowy",
        [STR_PARTLY] = "Partly cloudy", [STR_CLOUDY] = "Cloudy", [STR_UNKNOWN] = "No Connection",

        [STR_UNK_CALL_ID] = "Unknown", [STR_RES_CALL_ID] = "Restricted", [STR_PRV_CALL_ID] = "Private number", [STR_UNAV_CALL_ID] = "Unavailable",
        [STR_CALL_DCL] = "To decline, press to power button", [STR_CALL_ANS] = "To answer, slide up", [STR_INC_CALL] = "INCOMING CALL"
};

static const char* lang_ru[] = {
        [STR_PHONE] = "Телефон", [STR_MSG] = "Сообщения", [STR_CAMERA] = "Камера",
        [STR_CONTACTS] = "Контакты", [STR_MUSIC] = "Музыка", [STR_GALLERY] = "Галерия",
        [STR_BROWSER] = "S-Browser", [STR_SETTINGS] = "Настройки",  [STR_CALNOT] = "Календарь", [STR_MAIL] = "Autumn Mail",
        [STR_STORE] = "Autumn Store", [STR_CALCULATE] = "Калькулятор", [STR_GIT] = "GitBrowser", [STR_FILES] = "Файлы", [STR_WIKI] = "Wikipedia",


        [STR_JAN] = "Января", [STR_FEB] = "Февраль", [STR_MAR] = "Марта",
        [STR_APR] = "Апрель", [STR_MAY] = "Май", [STR_JUN] = "Июнь",
        [STR_JUL] = "Июль", [STR_AUG] = "Август", [STR_SEP] = "Сентябрь",
        [STR_OCT] = "Октябрь", [STR_NOV] = "Ноябрь", [STR_DEC] = "Декабрь",

        [STR_SUN] = "Воскресенье", [STR_MON] = "Понедельник", [STR_TUE] = "Вторник",
        [STR_WED] = "Среда", [STR_THU] = "Четверг", [STR_FRI] = "Пятница",
        [STR_SAT] = "Суббота"

        [STR_SUNNY] = "Солнечно", [STR_RAINY] = "Дождливый", [STR_SNOWY] = "Снежный",
        [STR_PARTLY] = "Переменная облачность", [STR_CLOUDY] = "Облачно", [STR_UNKNOWN] = "Облачно",

        [STR_UNK_CALL_ID] = "Неизвестный", [STR_RES_CALL_ID] = "Ограниченный", [STR_PRV_CALL_ID] = "Скрытый номер", [STR_UNAV_CALL_ID] = "Недоступно",
        [STR_CALL_DCL] = "Нажмите кнопку питания, чтобы отклонить", [STR_CALL_ANS] = "Прокрутите вверх, чтобы ответить", [STR_INC_CALL] = "ВХОДЯЩИЕ ВЫЗОВЫ"
};

static const char* lang_ch[] = {
        [STR_PHONE] = "電話", [STR_MSG] = "訊息", [STR_CAMERA] = "相機",
        [STR_CONTACTS] = "聯絡人", [STR_MUSIC] = "音樂播放器", [STR_GALLERY] = "畫廊",
        [STR_BROWSER] = "S-Browser", [STR_SETTINGS] = "系統設定",  [STR_CALNOT] = "日曆", [STR_MAIL] = "Autumn Mail",
        [STR_STORE] = "Autumn Store", [STR_CALCULATE] = "計算機", [STR_GIT] = "GitBrowser", [STR_FILES] = "文件管理器", [STR_WIKI] = "Wikipedia",


        [STR_JAN] = "一月", [STR_FEB] = "二月", [STR_MAR] = "三月",
        [STR_APR] = "四月", [STR_MAY] = "五月", [STR_JUN] = "六月",
        [STR_JUL] = "七月", [STR_AUG] = "八月", [STR_SEP] = "九月",
        [STR_OCT] = "十月", [STR_NOV] = "十一月", [STR_DEC] = "十二月",

        [STR_SUN] = "星期日", [STR_MON] = "週一", [STR_TUE] = "週二",
        [STR_WED] = "週三", [STR_THU] = "週四", [STR_FRI] = "星期五",
        [STR_SAT] = "週六"

        [STR_SUNNY] = "陽光明媚", [STR_RAINY] = "下雨的", [STR_SNOWY] = "Snowy",
        [STR_PARTLY] = "Partly cloudy", [STR_CLOUDY] = "Cloudy", [STR_UNKNOWN] = "No Connection",

        [STR_UNK_CALL_ID] = "未知", [STR_RES_CALL_ID] = "受限制號碼", [STR_PRV_CALL_ID] = "無主叫號碼", [STR_UNAV_CALL_ID] = "無法顯示",
        [STR_CALL_DCL] = "按電源鍵拒絕接聽", [STR_CALL_ANS] = "向上滑動即可回答。", [STR_INC_CALL] = "來電"
};

static const char* lang_hi[] = {
        [STR_PHONE] = "टेलीफ़ोन", [STR_MSG] = "संदेशों", [STR_CAMERA] = "कैमरा",
        [STR_CONTACTS] = "सम्पर्क", [STR_MUSIC] = "संगीत", [STR_GALLERY] = "गैलरी",
        [STR_BROWSER] = "S-Browser", [STR_SETTINGS] = "सेटिंग्स",  [STR_CALNOT] = "कैलेंडर", [STR_MAIL] = "Autumn Mail",
        [STR_STORE] = "Autumn Store", [STR_CALCULATE] = "कैलकुलेटर", [STR_GIT] = "GitBrowser", [STR_FILES] = "फ़ाइलें", [STR_WIKI] = "Wikipedia"


        [STR_JAN] = "जनवरी", [STR_FEB] = "फ़रवरी", [STR_MAR] = "मार्च",
        [STR_APR] = "अप्रैल", [STR_MAY] = "मई", [STR_JUN] = "जून",
        [STR_JUL] = "जुलाई", [STR_AUG] = "अगस्त", [STR_SEP] = "सितंबर",
        [STR_OCT] = "अक्टूबर", [STR_NOV] = "नवंबर", [STR_DEC] = "दिसंबर",

        [STR_SUN] = "रविवार", [STR_MON] = "सोमवार", [STR_TUE] = "मंगलवार",
        [STR_WED] = "बुधवार", [STR_THU] = "गुरुवार", [STR_FRI] = "शुक्रवार",
        [STR_SAT] = "शनिवार"

        [STR_SUNNY] = "धूप", [STR_RAINY] = "बारिश", [STR_SNOWY] = "बर्फबारी",
        [STR_PARTLY] = "आंशिक रूप से बादल", [STR_CLOUDY] = "बादल छाए रहेंगे", [STR_UNKNOWN] = "कोई कनेक्शन नहीं",

        [STR_UNK_CALL_ID] = "अज्ञात" [STR_RES_CALL_ID] = "प्रतिबंधित आईडी", [STR_PRV_CALL_ID] = "निजी नंबर", [STR_UNAV_CALL_ID] = "अनुपलब्ध",
        [STR_CALL_DCL] = "अस्वीकार करने के लिए पावर बटन दबा"एं [STR_CALL_ANS] = "उत्तर देने के लिए ऊपर स्वाइप करे", [STR_INC_CALL] = "आने वाली कॉल"
};
