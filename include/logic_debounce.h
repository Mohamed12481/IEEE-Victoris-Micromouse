#ifndef DEBOUNCE_H
#define DEBOUNCE_H

// ==========================================================
// منطق الـ debounce بالوقت - مستقل تمامًا عن أي هاردوير
// ==========================================================

// بيرجع true لو النبضة دي "حقيقية" (يعني عدّت وقت كافي من آخر نبضة مقبولة)
// وbترجع false لو النبضة دي غالبًا ارتداد ميكانيكي (bounce) ومينفعش نقبلها
bool debounce_should_accept(unsigned long now_ms, unsigned long last_accepted_ms, unsigned long debounce_window_ms);

#endif // DEBOUNCE_H