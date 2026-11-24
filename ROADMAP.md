# نقشهٔ راه Workledge

## فاز ۰ — آمادهسازی محیط (انجام شد)

- [x] کلون Workrave (`main`, `e055e2ff0`)
- [x] کلون Dynamic Ledge (`9c5dd4d`, نسخه 1.5.0)
- [x] کاهش حجم workspace: حذف تاریخچهٔ کامل Workrave (۱۲۷ مگ → ۸۱ کیلوبایت متادیتا)، حفظ ۱۶۰۶ فایل سورس
- [x] تعیین ویندوز به عنوان هدف اصلی و Qt6/QML به عنوان لایهٔ رابط مشترک

## فاز A — MVP ویندوز-اول

**هدف:** یک اپ واحد که تایمر Workrave را داخل یک پیل شناور بالای صفحه نشان دهد.

| کار | فایل/محل | معیار پذیرش |
|---|---|---|
| ساخت کامپوننت QML پیل/پنل | `ui/app/toolkits/qt/qml/NotchHud.qml` | حالت جمع/باز با انیمیشن، نمایش micro/rest/daily |
| Bridge وضعیت تایمر | `NotchHudBridge.{hh,cc}` | مقادیر زمان و پیشرفت از `StatusWindowBridge`/core میآیند، نه عدد ثابت |
| پنجرهٔ شناور | `NotchHudWindow.{hh,cc}` | `FramelessWindowHint + WindowStaysOnTopHint + Tool`, وسط بالای صفحه، شفاف |
| اتصال به چرخهٔ زندگی اپ | `Toolkit.cc` | همزمان با `MainWindow` ساخته و با بسته شدن اپ پاک میشود |
| تنظیمات | `ui/app/include/ui/GUIConfig.hh`, `GUIConfig.cc` | کلیدهای `gui/notch_hud/enabled`, `/position`, `/width` |
| دکمهها | Bridge signals | «Break now» → core، «Later» → postpone، «Settings» → `QmlPrefsDialog` |
| بیلد ویندوز | `CMakeLists.txt` (تنظیمات پیشفرض موجود است) | `-DWITH_UI=Qt` بدون خطا بیلد میشود |

**خروجی فاز A:** Workrave با ظاهر «Dynamic Ledge Pill» روی ویندوز؛ همان بیلد روی مک/لینوکس هم قابل اجرا است (بدون قابلیتهای اختصاصی مک).

## فاز B — همسانی قابلیتها (انتخابی)

از لیست `FEATURE_MAP.md`، مواردی که روی ویندوز معنی دارند و هزینهٔ متوسطی دارند:

- [ ] نوار باتری / صدا / روشنایی (Windows API)
- [ ] کلیدبورد/clipboard history ساده (`QClipboard`)
- [ ] System stats سبک
- [ ] Keep Awake (`SetThreadExecutionState`)
- [ ] Mirror دوربین (`QCamera`)
- [ ] تمهای کوچک/متوسط/بزرگ و Glass در QML
- [ ] فشردهسازی فایل / ادغام PDF (کتابخانههای Qt + Zip)

**توضیح:** «Now playing + lyrics» و «تشخیص جلسات» نیاز به بکاند WinRT/SMTC و تقویم ویندوز دارند؛ اینها جزء فاز Cاند.

## فاز C — پالایش کراسپلتفرم

- **لینوکس:** Workrave از قبل `WaylandLayerShell` دارد؛ همان مکانیسم را برای HUD استفاده میکنیم تا پنجره واقعاً شبیه نوتچ در لایهٔ Wayland رفتار کند.
- **مک:** از `platforms/macos/MacOSOverlayWindow.mm` موجود استفاده میشود تا پیل واقعاً روی نوتچ قرار گیرد.
- **ویندوز:** بهبود Drift/DPI و multimonitor، و در صورت تمایل WinRT SMTC برای موزیک.

## ریسکها

1. Bridge باید با مدل قدیمی `StatusWindowBridge` هماهنگ شود؛ در غیر اینصورت وضعیت تایمر دوتایی میشود.
2. پنجرهٔ always-on-top روی ویندوز گاهی توسط تمامصفحهها پوشانده میشود؛ نیاز به تنظیم `QSurfaceFormat` و.refresh periodic دارد.
3. برخی قابلیتهای Dynamic Ledge ماهیتاً مک-specificاند و فقط «معادل نزدیک» دارند، نه کپی دقیق.

## وضعیت فعلی پیادهسازی

فاز A از نظر طراحی و اسکلت رابط آماده است، ولی **کامپایل نشده** چون ابزار CMake/Qt در sandbox موجود نیست. مرحلهٔ بعدی پس از تأمین Qt6 و CMake، اتصال Bridge به core و تست بیلد ویندوز است.
