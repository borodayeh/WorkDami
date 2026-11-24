# نمونهٔ رابط Notch HUD

`NotchHud.qml` یک کامپوننت مستقل است که حالتهای **جمعشده (پیل)** و **باز (پنل)** دارد و دادهها را یا از پراپرتیهای محلی میگیرد یا از یک `bridge` خارجی (مدل `StatusWindowBridge` در Workrave).

## پیشنمایش بدون بیلد Workrave

```bash
# از داخل workledge/
qmlscene prototype/NotchHudPreview.qml
# یا
qml prototype/NotchHudPreview.qml
```

روی پیل کلیک کنید تا باز شود. دکمهها فقط سیگنال صادر میکنند؛ در پیشنمایش پیام وضعیت پایین صفحه تغییر میکند.

## اتصال به Workrave

1. `NotchHud.qml` را داخل `workrave/ui/app/toolkits/qt/qml/` کپی کنید.
2. در `qml/sanctuary.qrc` خط زیر را اضافه کنید:

   ```xml
   <file>NotchHud.qml</file>
   ```

3. `NotchHudBridge.hh` الگویی برای QObjectای است که باید به core Workrave وصل شود. در فایل واقعی، TODOها را با فراخوانیهای `IAppCore` / `StatusWindowBridge` پر کنید.
4. در `QmlTimerBoxView.cc` میتوانید از همان الگوی `setContextProperty("bridge", ...)` برای تزریق Bridge به QML استفاده کنید.

## نکته

این فایلها صرفاً رابط و قرارداد دادهاند؛ منطق تایمر هنوز در Workrave است و کپی نشده است.
