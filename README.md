# Workledge — نام کاری

> **ساخت قابل بازتولید (جدید):** `.github/workflows/windows-qt.yml` + `scripts/` + `patches/workrave/`. جزئیات در `integration/README.md`. تا اجرای workflow، بیلد ویندوز **تأییدنشده** است.

> هدف: **یک نرم افزار واحد، ویندوز-اول** که موتور تایمر/استراحت Workrave را با تجربهٔ رابط «نوتچ/پیل» Dynamic Ledge ترکیب کند. مک و لینوکس هم پوشش داده میشوند، ولی اولویت با ویندوز است.

## وضعیت فعلی

این پوشه یک **طرح یکپارچگی + نمونهٔ اولیه رابط** است، نه یک بیلد نهایی. دو ریپو در workspace کلون شدهاند:

| ریپو | مسیر | نقش در محصول نهایی |
|---|---|---|
| Workrave | `../workrave` | **پایهٔ کد**: C++ + Qt6/QML، کراسپلتفرم، موتور تایمر و تنظیمات |
| Dynamic Ledge | `../dynamic_ledge` | **مرجع طراحی/رفتار**: SwiftUI + macOS 14 فقط |

### چرا Workrave به عنوان پایه؟

- از قبل ویندوز، لینوکس و مک را پشتیبانی میکند (`PLATFORM_OS_WINDOWS` / `UNIX` / `MACOS`).
- گزینهٔ رسمی `-DWITH_UI=Qt` دارد و رابط QML مدرن آن (`sanctuary.qrc`, `StatusWindowBridge`) آماده است.
- مجوز هردو پروژه **GPL-3.0** است، پس ترکیبشان از نظر مجوز مجاز است به شرطی که خروجی هم GPL بماند.

### چرا کد Swift را مستقیم پورت نمیکنیم؟

Dynamic Ledge فقط macOS 14+ و SwiftUI است و به توابع اختصاصی مک (notch, WindowServer, MediaBridge) وابسته است. اجرای آن روی ویندوز عملاً غیرممکن است. به جای آن، **رفتار و ظاهر** آن را در Qt/QML باز پیاده میکنیم و وسعت قابلیتها را مرحلهای اضافه میکنیم. کد Swift به عنوان مشخصات رفتاری (`FEATURE_MAP.md`) باقی میماند.

## ساختار این پوشه

```text
workledge/
├── README.md          # همین فایل
├── ROADMAP.md         # فازها و معیار پذیرش
├── FEATURE_MAP.md     # نگاشت قابلیتهای Dynamic Ledge به Qt/Workrave
└── prototype/
    ├── README.md
    ├── NotchHud.qml        # کامپوننت رابط پیل/نوتچ (قابل استفاده مجدد)
    ├── NotchHudPreview.qml # پیشنمایش مستقل بدون Bridge
    └── NotchHudBridge.hh   # اسکلت QObject bridge (الگو برای پیادهسازی C++)
```

## شروع کار

### 1) پیشنمایش رابط (بدون بیلد Workrave)

اگر Qt6 Console/Native tools نصب است:

```bash
qmlscene prototype/NotchHudPreview.qml
# یا
qml prototype/NotchHudPreview.qml
```

### 2) بیلد ویندوز پیشنهادی Workrave (پایه)

```powershell
cmake -S workrave -B workrave/build ^
  -DWITH_UI=Qt ^
  -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>\scripts\buildsystems\vcpkg.cmake

cmake --build workrave/build --config Release
```

> **توجه:** در محیط فعلی `cmake`، `qmake6` و `swift` نصب نیستند؛ فقط `g++` موجود است. بنابراین کدهای رابط نوشته شدند ولی **در اینجا کامپایل و اجرا نشدند**. برای تست واقعی باید Qt6 + CMake + vcpkg نصب شوند.

## مراحل الحاق کد

1. فایل `prototype/NotchHud.qml` را به `workrave/ui/app/toolkits/qt/qml/` کپی کنید.
2. آنرا در `workrave/ui/app/toolkits/qt/qml/sanctuary.qrc` اضافه کنید.
3. `NotchHudBridge` را طبق الگوی `QmlTimerBoxView.hh` به وضعیت واقعی core وصل کنید.
4. پنجرهٔframeless و always-on-top را در `Toolkit.cc` در کنار `MainWindow` بسازید.
5. تنظیمات `gui/notch_hud/*` را به `GUIConfig` اضافه کنید.

جزئیات در [ROADMAP.md](ROADMAP.md) و [FEATURE_MAP.md](FEATURE_MAP.md) است.

## مجوز

Workrave و Dynamic Ledge هردو GPL-3.0اند. تمام کد جدید باید تحت GPL-3.0 باقی بماند و نام دو پروژهٔ اصلی در توضیحات حفظ شود.
