# گرامر B-Minor

گرامر رو با EBNF نوشتم. Parser پروژه بازگشتی نزولی و LL(1)ـه و برای هر قاعده یه تابع داره.

نمادها: `{ x }` یعنی صفر بار یا بیشتر، `[ x ]` یعنی اختیاری، `|` یعنی «یا»، و چیزهای داخل کوتیشن همون‌طور که هستن توی کد میان. `IDENT`، `INT`، `CHAR`، `STRING` و `EOF` هم نوع توکن‌هایی هستن که Lexer می‌سازه.

## قواعد

```
program      ::= { declaration } EOF

declaration  ::= IDENT ':' ( func_type ( ';' | '=' block )
                           | plain_type [ '=' initializer ] ';' )

type         ::= plain_type | func_type
plain_type   ::= basic_type | 'array' '[' [ expr ] ']' type
func_type    ::= 'function' type '(' [ param { ',' param } ] ')'
basic_type   ::= 'integer' | 'boolean' | 'char' | 'string' | 'void' | 'auto'
param        ::= IDENT ':' type

initializer  ::= array_init | expr
array_init   ::= '{' [ init_elem { ',' init_elem } ] '}'
init_elem    ::= array_init | expr

statement    ::= declaration | if_stmt | for_stmt | while_stmt
               | print_stmt | return_stmt | block | expr_stmt
block        ::= '{' { statement } '}'
if_stmt      ::= 'if' '(' expr ')' statement [ 'else' statement ]
for_stmt     ::= 'for' '(' [ expr ] ';' [ expr ] ';' [ expr ] ')' statement
while_stmt   ::= 'while' '(' expr ')' statement
print_stmt   ::= 'print' [ expr { ',' expr } ] ';'
return_stmt  ::= 'return' [ expr ] ';'
expr_stmt    ::= expr ';'

expr         ::= assignment
assignment   ::= logic_or [ '=' assignment ]
logic_or     ::= logic_and { '||' logic_and }
logic_and    ::= comparison { '&&' comparison }
comparison   ::= additive { ( '<' | '<=' | '>' | '>=' | '==' | '!=' ) additive }
additive     ::= term { ( '+' | '-' ) term }
term         ::= power { ( '*' | '/' | '%' ) power }
power        ::= unary [ '^' power ]
unary        ::= ( '-' | '!' ) unary | postfix
postfix      ::= primary { '[' expr ']' | '++' | '--' }
primary      ::= INT | CHAR | STRING | 'true' | 'false'
               | IDENT [ '(' [ arg_list ] ')' ]
               | '(' expr ')'
arg_list     ::= expr { ',' expr }
```

## اولویت عملگرها

از بالا (قوی‌تر) به پایین:

```
f(...)  a[i]  x++  x--
-  !                   (یگانی)
^                      (راست به چپ)
*  /  %
+  -
<  <=  >  >=  ==  !=
&&
||
=                      (راست به چپ)
```

اولویت از خود ترتیب قاعده‌ها میاد: هر قاعده فقط از قاعده‌ی بالاتر خودش استفاده می‌کنه، برای همین `a + b * c` می‌شه `a + (b * c)`.

## حذف بازگشت چپ

قاعده‌ی طبیعی `additive` این‌طوریه:

```
additive ::= additive ( '+' | '-' ) term | term
```

این بازگشت چپ داره و اگه همین رو توی Parser بنویسیم، تابع `additive` بی‌نهایت خودش رو صدا می‌زنه. قاعده‌ی کلی اینه که `A ::= A α | β` رو به `A ::= β { α }` تبدیل کنیم:

```
additive ::= term { ( '+' | '-' ) term }
```

که توی کد می‌شه یه حلقه‌ی `while`. همین کار رو برای `logic_or`، `logic_and`، `comparison`، `term` و `postfix` هم کردم. `assignment` و `power` بازگشت راست دارن که برای Parser مشکلی نیست و راست به چپ بودنشون رو هم می‌ده.

## بررسی LL(1)

تقریباً همه‌جا Parser با دیدن یه توکن می‌فهمه کدوم گزینه رو بره. مثلاً بعد از `IDENT ':'` اگه توکن بعدی `function` باشه تعریف تابعه، و اگه `integer` یا `array` و مثل اون‌ها باشه، متغیره (مجموعه‌ی FIRST این دو تا از هم جداست). همین‌طور بعد از `=` اگه `{` ببینه آرایه است و در غیر این صورت عبارت. حلقه‌های عبارت‌ها هم تعارض ندارن، چون مثلاً `+` و `-` جزو توکن‌های بعد از `additive` نیستن.

دو جا مشکل داشت:

**شروع دستور با `IDENT`.** هم اعلان (`x: integer = 1;`) و هم عبارت (`x = 1;`) با `IDENT` شروع می‌شن. برای تشخیص، توکن دوم رو هم نگاه می‌کنم: اگه `:` باشه اعلانه. چون هیچ عبارتی `:` نداره، ابهامی نمی‌مونه. پس همین یه جا LL(2) هست.

**else معلق.** توی `if (a) if (b) x; else y;` مشخص نیست `else` مال کدوم `if` هست. مثل بقیه‌ی زبان‌ها اونو به نزدیک‌ترین `if` می‌چسبونم، که توی Parser خودبه‌خود همین اتفاق می‌افته (بعد از دستور داخلی، اگه `else` باشه همون‌جا مصرفش می‌کنیم).

یه چیز دیگه: گرامر سمت چپ `=` رو هر `logic_or` می‌گیره، ولی Parser بعدش چک می‌کنه که فقط متغیر یا `a[i]` باشه و گرنه خطای `invalid assignment target` می‌ده.

## فرض‌هایی که کردم

- `while` کلیدواژه است.
- `^` راست‌به‌چپه و از `-` یگانی ضعیف‌تره، یعنی `-2 ^ 2` برابر `(-2)^2` حساب می‌شه.
- اعداد توی Lexer علامت ندارن. `-` همیشه عملگره و Parser تصمیم می‌گیره یگانی باشه یا دودویی.

## بازیابی خطا

بعد از یه خطای نحوی، Parser توکن‌ها رو دور می‌ندازه تا به یکی از این نقطه‌ها برسه و از اونجا ادامه می‌ده: بعد از یه `;`، قبل از `{` یا `}`، قبل از `if`، `for`، `while`، `print`، `return`، یا قبل از شروع یه اعلان جدید (`IDENT` و بعدش `:`).
