# نگاشت قابلیتها: Dynamic Ledge → Workledge (Qt/Windows)

| قابلیت Dynamic Ledge | معادل در Workledge | سختی | فاز |
|---|---|---|---|
| نوتچ / Pill شناور | `QQuickWindow` frameless + always-on-top، وسط بالا (پیل شناور) | متوسط | A |
| تایمر micro/rest/daily | مستقیم از Workrave core + `StatusWindowBridge` | آسان | A |
| Break exercises | QML موجود Workrave (`ExercisesDialog.qml`) | آسان | A |
| شروع استراحت / بهتعویق / رد | signalها از Bridge به `IAppCore` | آسان | A |
| تم کوچک/متوسط/بزرگ + Glass | پارامترهای QML + تنظیمات | آسان | A/B |
| Keep Awake | `SetThreadExecutionState` (Win32) | آسان | B |
| System stats | Win32 PerfCounter / Qt، نمایش در پنل | آسان | B |
| باتری / صدا / روشنایی | `GetSystemPowerStatus`, Core Audio/WMI | متوسط | B |
| Clipboard history | `QClipboard` + ذخیرهٔ محلی (OCR اختیاری) | متوسط | B |
| Mirror دوربین | `QCamera` + QML VideoOutput | متوسط | B |
| Tray فایلها + Zip/merge PDF | Qt + bookkeeping + کتابخانهٔ zip | متوسط | B |
| Now playing + lyrics + آهنگ | WinRT SMTC (Windows) / MPRIS (Linux) / MediaPlayer (macOS) | سخت | C |
| جلسات (FaceTime/Zoom/Meet…) | WinRT notifications + calendar integration | سخت | C |
| تقویم و یادآورها | Windows App SDK Calendar API | سخت | C |
| AirDrop | معادل ویندوز وجود ندارد؛ Share/network nearby جایگزین | سخت/غیرممکن 1:1 | C |
| Shortcuts | Qt/QML actions + Windows registered actions | متوسط | C |
| لاگین/حریم خصوصی | Workrave از قبل local-first است؛ حفظ همان مدل آفلاین | آسان | A |

## اصول نگاشت

- هر جا Dynamic Ledge از API اختصاصی macOS استفاده میکند، Workledge از **پشت بکاند پلتفرمی** در Qt/C++ استفاده میکند تا کد QML یکسان بماند.
- قابلیتی که روی ویندوز معنایی ندارد (AirDrop، notch فیزیکی) حذف یا با معادل نزدیک جایگزین میشود.
- ترتیب اضافه کردن قابلیتها بر اساس «تاثیر تجربی / هزینهٔ پیادهسازی» است، نه ترتیب ظاهر شدن در ریپو Swift.
