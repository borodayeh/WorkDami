# بستهٔ تغییرات اتصال (تأییدنشده)

این فایلها **در هستهٔ تستشده گنجانده نشدهاند** و هنوز کامپایل نشدند.

- `workrave_exercise_bridge.h/.cc` — تبدیل واقعی `ExerciseViewDto` به `workrave::Exercise` (شامل `commonui/Exercise.hh`). فیلد source/license/attribution/reviewStatus از طریق `BridgedExercise::source` حفظ میشود.
- `workrave_config_store.h/.cc` — بکاند محصول با `config/IConfigurator.hh` و کلیدهای پایدار `gui/break_exercise/...`.

## مسدودکنندهها

1. `IConfigurator::set_value` و `save()` در این نسخه `void`اند؛ موفقیت ذخیره قابل تشخیص نیست. تست یکپارچه لازم است.
2. تبدیل بطرف Workrave فقط با include paths و لینک/libs مربوطه قابل کامپایل است.
3. رابط Qt و رویداد واقعی استراحت هنوز وصل نشدند.

## دستور محلی (بدون مسیر مطلق)

از ریشهٔ مخزن `workledge`:

```bash
bash scripts/fetch_upstream.sh
bash scripts/run_core_tests.sh build/core
bash scripts/msys2_build.sh upstream/workrave build/workrave-qt   # فقط در MSYS2 CLANG64
bash scripts/apply_integration.sh upstream/workrave
bash scripts/msys2_build.sh upstream/workrave build/workrave-patched
bash scripts/package_windows.sh build/workrave-patched artifacts/maksa-test-fixture upstream/workrave
```

هیچ مسیر `/home/user/...` در اسکریپتها، patch یا workflow وجود ندارد.


## منبع قابل بازتولید

- upstream: `upstream/workrave.txt` → `e055e2ff0cb2cb68f60a86137048608459aec2cc`
- تغییرات ما: `patches/workrave/0001-maksa-integration.patch`
- progenyها: workflow ابتدا upstream را میگیرد، Workrave پایه را میسازد، سپس `core/` و `integration/` را کپی و patch را اعمال میکند.
- Dynamic Ledge فقط مرجع طراحی است و در ساخت فراخوانی نمیشود.


## تصمیم نهایی مرحله

نصب برنامهٔ لینوکسی پیشنهادی توسط کاربر **تأیید نشد**. هیچ بستهای نصب نشد، هیچ سرویس خارجی تغییری نکرد و بیلد Workrave/QML آغاز نشد. بنابراین:

- ۷۰ تست مستقل هسته معتبراند (۲۸ + ۴۲).
- **صفر تست یکپارچه** با Workrave وجود دارد.
- مسیر پذیرش داخل اپ اجرا نشده و نباید موفق اعلام شود.
- اولین خطای مسدودکننده همان `cmake: command not found` است؛ حتی با نصب لینوکسی، Boost 1.83 در برابر الزام 1.86 و نبود DISPLAY باقی میماند.

