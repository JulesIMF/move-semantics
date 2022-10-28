# К вопросу о копировании
Эта небольшая статья-исследование посвящена одной важной теме. касающейся языка C++ -- проблеме излишнего копирования и способам борьбы с ним. Излишнего копирования старались избегать всегда, даже в старом добром Си -- например, передавая объект в функцию по указателю или по ссылке в С++, но ряд проблем так решить не удавалось. В частности, не очень эффективно копировать временные объекты, зная, что им, в общем-то, осталось жить всего ничего. Разумеется, уже существует множество статей про семантику перемещения и описание rvalue-ссылок. Рекомендую ["Краткое введение в rvalue-ссылки"](https://habr.com/ru/post/226229/), ["Подробное введение в rvalue-ссылки для тех, кому не хватило краткого"](https://habr.com/ru/post/322132/) и, внезапно, ["MSDN: value categories. Lvalues and Rvalues (C++)"](https://docs.microsoft.com/en-us/cpp/cpp/lvalues-and-rvalues-visual-cpp?view=msvc-160). 

В этой статье мы попробуем эаолюционно, шаг за шагом, прийти к пониманию того, откуда взялась эта семантика перемещений и почему она устроена именно так. Но для того, чтобы не быть голословными, нам нужен какой-нибудь инструмент для исследования.

## Tracker
Для этих целей была написана небольшая библиотека ([реализация на GitHub](https://github.com/JulesIMF/move-semantics/tree/master/Tracker)), которая позволяет отслеживать весь цикл жизни объекта, в том числе и копирования. Для того, чтобы использовать ее, необходимо вставить в методы класса (такие как конструкторы, операторы присваивания и деструктор) функции, уведомляющие главный трекер об интересном событии. Трекер -- это, собственно, инструмент, который собирает статистику и выводит ее в удобочитаемом формате. В этой статье мы будем проводить исследования с использованием класса Int, который ведет себя как целое число (и является, конечно, оберткой над ним), поэтому в его методы вставлены необходимые вызовы трекера. Например, копирующий конструктор выглядит так:
```cpp
Int(Int const& a, std::string name = "") :
    value(a.value)
{
    Tracker::mainLogger.setName(info, name);
    info.value = std::to_string(value);
    info.address = this;
    Tracker::mainLogger.enterCTORCopy(info, a.info);
}
```

Чтобы при чтении логов понимать, с каким объектом мы имеем дело, надо выводить его имя (если оно,конечно, есть -- временные объекты имен не имеют). Но откуда его брать? Идеальным решением был бы поиск этого имени из файлов с отладочной информацией, но это значительно усложнило бы реализацию. Поэтому это имя придется передавать в объект прямо при его создании. Если автор класса наделил свои конструкторы дополнительным параметором:
```cpp
MyClass::MyClass(..., std::string name = "");
```
то можно воспользоваться макросом `TRACKER_CREATE`, чтобы не вбивать вручную имя, например:
```cpp
TRACKER_CREATE(MyClass, awesome_variable, 42);
```

После того, как отслеживаемый класс написан, с ним можно и поиграться. На самом деле mainLogger -- это всего лишь диспетчер, представленный в виде глоабльного объекта, который пересылает информацию настоящим логгерам. Сделано это для того, чтобы лог можно было получать одновременно в разных форматах -- в виде консольного вывода, в HTML-файле или как граф.

Давайте попробуем сделать с Int'ами что-нибудь несложное, чтобы познакомиться с форматом вывода. Сразу скажу, что граф -- очень наглядная структура, но картинки занимают много места, поэтому в дальнейшем в сложных примерах я буду использовать текстовые логи.

Запустим такой код (для определённости скажу, что все дальнейшее запускается на Linux и компилируется Clang 12.0):
```cpp
Int extraCopyIncrement(Int arg)
{
    // Пример кажется слишком простым, но в ином случае
    // будет слишком много текста, и разбирать его станет нереально
    return arg++;
}

int main()
{
    TRACKER_DEFAULT_INITIALIZATION; // этот макрос используется один раз в самом начале main
    TRACKER_ENTER; // а этот макрос вставляется в начале функции, чтобы позволить отследить вход в функцию
    TRACKER_CREATE(Int, object, 0);
    TRACKER_CREATE(Int, returnValue, extraCopyIncrement(object));
}
```

Когда он отработает, в логах мы получим вот такой вывод:
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Tue Mar  8 23:26:04 2022


int main()
{
    <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a490</font></b>)
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a5d0</font></b>) from <b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a490</font></b>)
    Int extraCopyIncrement(Int)
    {
        Int Int::operator++(int)
        {
            <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a340</font></b>) from <b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a5d0</font></b>)
            Int& Int::operator++()
            {
            }
            <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a670</font></b>) from <b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a340</font></b>)
            <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a340</font></b>)
        }
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValue" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a530</font></b>) from <b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a670</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a670</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a5d0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValue" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a530</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c053a490</font></b>)
}


5 objects created, 4 copied, 0 moved
</span></pre>
<img src="img/tracker_1.png">

Отлично, как мы видим, при передаче `object` в функцию появился временный объект `tmp1` (который реально называется `copy`). Потом у `tmp1` вызывается постфиксный инкремент, внутри которого создается еще один временный объект `currentValueCopy`. При возвращении из этого инкремента `currentValueCopy` копируется в `tmp3`, а `tmp3` копируется в `returnValue` при выходе из `extraCopyIncrement`.

## Copy elision
Да... Надо признаться, уже здесь видно, что не все так хорошо. Зачем нужно это копирование сначала из `currentValueCopy` в `tmp2`, а потом из `tmp2` в `returnValue`. Неужели нельзя этого избежать?

Можно! Гениальные системные программисты придумали "copy elision" -- оптимизацию, которая удаляет лишние копирования. Если говорить точнее, то в данном случае нас интересует важнейшее её проявление "return value optimisation" (RVO). Смысл её заключается в том, что возвращаемое значение записывается прямо в конечный приемник без вызова лишних копирований. Достигается эта оптимизация примерно так. Допустим, имеем функцию с такой сигнатурой:
```cpp
SomeType function(int arg);
```

Компилятор при генерации кода решает, что можно эту функцию переделать в такую:
```cpp
void function(SomeType* returned, int arg);
```
И просто при вызове этой функции первым параметром передает адрес, куда нужно записать ответ. Теперь функции известно, куда записать ответ!

На самом деле, начиная со стандарта C++17, RVO относится к категории "Mandatory elision of copy/move operations", то есть к обязательным. Для того, чтобы показать, что происходит в её отсутствие, я насильно подавил эту оптимизацию с помощью флага `-fno-elide-constructors`. Давайте теперь вернём её обратно и посмотрим, что получилось:
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 00:47:01 2022


int main()
{
    <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdd70</font></b>)
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdeb0</font></b>) from <b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdd70</font></b>)
    Int extraCopyIncrement(Int)
    {
        Int Int::operator++(int)
        {
            <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdf50</font></b>) from <b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdeb0</font></b>)
            Int& Int::operator++()
            {
            }
        }
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValue" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcde10</font></b>) from <b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdf50</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdf50</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdeb0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValue" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcde10</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>c9bcdd70</font></b>)
}


4 objects created, 3 copied, 0 moved
</span></pre>

Так, теперь у нас на одно копирование меньше! Это хороший прогресс, но все-таки от копирования `currentValueCopy` в `returnValue` мы так и не избавились. Это логично, ведь в `Int Int::operator++(int)` сначала создается копия текущего состояния, а только потом вызывается оператор префиксного инкремента:

```cpp
Int operator++(int)
    {
        TRACKER_ENTER;
        TRACKER_CREATE(Int, currentValueCopy, *this);
        ++(*this);
        return currentValueCopy;
    }
```

Компилятор смог бы избежать этого копирования, если бы проследил, что все, зачем нужен объект `currentValueCopy` -- это просто быть возвращённым. Но, к сожалению, у него этого не получилось сделать((

Конечно, можно поправить код этого оператора, чтобы не создавать этой копии, ведь можно просто вернуть новый экземпляр `Int` с уменьшенным на один значением, тогда он, вероятно, создаст этот экземпляр прямо на месте `returnValue`. Давайте попробуем, вдруг у него теперь получится:

```cpp
Int operator++(int)
    {
        TRACKER_ENTER;
        ++(*this);
        return Int(value - 1);
    }
```

<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 01:06:45 2022


int main()
{
    <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa138fc0</font></b>)
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa139100</font></b>) from <b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa138fc0</font></b>)
    Int extraCopyIncrement(Int)
    {
        Int Int::operator++(int)
        {
            Int& Int::operator++()
            {
            }
            <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa1391a0</font></b>)
        }
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValue" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa139060</font></b>) from <b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa1391a0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa1391a0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa139100</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValue" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa139060</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"object" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>aa138fc0</font></b>)
}


4 objects created, 2 copied, 0 moved
</span></pre>

Замечательно, мы избавились от одного копирования, заменив его на создание нового объекта! То есть наше предположение, что объект будет создан прямо на месте `returnValue`, не оправдалось. И слава Богу, ведь писать такой оператор инкремента -- это чудовищно)

Можно ещё немного поиграться с этой оптимизацией и попробовать понять: а важно ли, насколько глубоко из стека будет подниматься возвращаемое значение? Влияет ли это на работу оптимизации? Попробуем вызвать несколько функций друг из друга и посмотрим на вывод: 
```cpp
Int level1()
{
    TRACKER_ENTER;
    return Int{};
}

Int level2()
{
    TRACKER_ENTER;
    return level1();
}

Int level3()
{
    TRACKER_ENTER;
    return level2();
}

int main()
{
    TRACKER_DEFAULT_INITIALIZATION;
    TRACKER_ENTER;
    TRACKER_CREATE(Int, returnValueLevel1, level1());
    TRACKER_CREATE(Int, returnValueLevel2, level2());
    TRACKER_CREATE(Int, returnValueLevel3, level3());
}
```
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 01:13:59 2022


int main()
{
    Int level1()
    {
        <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e470</font></b>) from <b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
    Int level2()
    {
        Int level1()
        {
            <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
        }
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e510</font></b>) from <b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
    Int level3()
    {
        Int level2()
        {
            Int level1()
            {
                <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
            }
        }
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel3" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e5b0</font></b>) from <b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e650</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel3" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e5b0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e510</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>11f9e470</font></b>)
}


6 objects created, 3 copied, 0 moved
</span></pre>
Как мы видим, компилятор не умеет создавать объект прямо на месте конечного хранилища: на строке NN лога надпись `CTOR "tmp3"` говорит о том, что компилятор создает временную переменную `tmp3`, ... Зато он может проследить, куда идет это возвращаемое значение, независимо от глубины рекурсии (что не удивительно, ведь адрес, где ждут возвращаемое значение, можно передавать сколько угодно раз). Но внезапно при отключении RVO ничего не меняется! А если посмотреть на почти такой же код, но когда мы создаем именованный объект и сразу же его возвращаем?

```cpp
Int level1()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel1, 0);
    return valueInLevel1;
}

Int level2()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel2, level1());
    return valueInLevel2;
}

Int level3()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel3, level2());
    return valueInLevel3;
}

int main()
{
    TRACKER_DEFAULT_INITIALIZATION;
    TRACKER_ENTER;
    TRACKER_CREATE(Int, returnValueLevel1, level1());
    TRACKER_CREATE(Int, returnValueLevel2, level2());
    TRACKER_CREATE(Int, returnValueLevel3, level3());
}
```

<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 01:36:19 2022


int main()
{
    Int level1()
    {
        <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>)
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c2d0</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>)
    Int level2()
    {
        Int level1()
        {
            <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c1b0</font></b>)
        }
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c1b0</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c1b0</font></b>)
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel2" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c370</font></b>) from <b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>)
    Int level3()
    {
        Int level2()
        {
            Int level1()
            {
                <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c070</font></b>)
            }
            <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c1b0</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c070</font></b>)
            <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c070</font></b>)
        }
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>7</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>) from <b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c1b0</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c1b0</font></b>)
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel3" </font></b>(id: <b><font color=#d670d6>8</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c410</font></b>) from <b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>7</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>7</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c4b0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel3" </font></b>(id: <b><font color=#d670d6>8</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c410</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel2" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c370</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>1402c2d0</font></b>)
}


9 objects created, 6 copied, 0 moved
</span></pre>

Кажется, что RVO не так уж хорошо справляется с подобными проблемами... Но в отсутствие этой оптимизации (`-fno-elide-constructors`) все было бы еще хуже:
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 01:40:17 2022


int main()
{
    Int level1()
    {
        <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>)
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>)
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel1" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2af70</font></b>) from <b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>)
    Int level2()
    {
        Int level1()
        {
            <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ac50</font></b>)
            <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ac50</font></b>)
            <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ac50</font></b>)
        }
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2adc0</font></b>) from <b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>)
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>) from <b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2adc0</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2adc0</font></b>)
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel2" </font></b>(id: <b><font color=#d670d6>7</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b010</font></b>) from <b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>)
    Int level3()
    {
        Int level2()
        {
            Int level1()
            {
                <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>8</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2aa40</font></b>)
                <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp4" </font></b>(id: <b><font color=#d670d6>9</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ac50</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>8</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2aa40</font></b>)
                <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>8</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2aa40</font></b>)
            }
            <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>10</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2abb0</font></b>) from <b><font color=#f5f543>"tmp4" </font></b>(id: <b><font color=#d670d6>9</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ac50</font></b>)
            <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp4" </font></b>(id: <b><font color=#d670d6>9</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ac50</font></b>)
            <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp5" </font></b>(id: <b><font color=#d670d6>11</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>) from <b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>10</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2abb0</font></b>)
            <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>10</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2abb0</font></b>)
        }
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>12</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2adc0</font></b>) from <b><font color=#f5f543>"tmp5" </font></b>(id: <b><font color=#d670d6>11</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp5" </font></b>(id: <b><font color=#d670d6>11</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2ae60</font></b>)
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>13</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>) from <b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>12</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2adc0</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>12</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2adc0</font></b>)
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"returnValueLevel3" </font></b>(id: <b><font color=#d670d6>14</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b0b0</font></b>) from <b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>13</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>13</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b150</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel3" </font></b>(id: <b><font color=#d670d6>14</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b0b0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel2" </font></b>(id: <b><font color=#d670d6>7</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2b010</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel1" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>50d2af70</font></b>)
}


15 objects created, 12 copied, 0 moved
</span></pre>

Как можно заметить, количество копирований увеличилось в два раза. Без этой оптимизации при каждом возврате из функции создается временный объект `tmp...`, который потом копируется в именованный объект и приказывает долго жить.

Поэтому можно сделать такой **вывод: RVO является очень полезной оптимизацией, которая в некоторых случаях значительно снижает количество лишних копирований без нашего вмешательства, но её явно недостаточно, чтобы убрать лишние копирования.** Необходим ещё какой-нибудь механизм!

## Move-семантика
Честно говоря, Int выступает не очень хорошим примером для этой проблемы: копирование Int\`a достаточно дешевая операция, чтобы можно было успокоиться и не лезть дальше. Поэтому давайте временно представим, что мы имеем дело с каким-нибудь контейнером, например, аналогом `std::string`:
```cpp
class String
{
public:
    String(String const& string) :
        size_(string.size_),
        data_(new char[size_]) //, ...
    {
        for (size_t idx = 0; string.data_[idx]; idx++)
            data_[idx] = string.data_[idx];

        data_[size_] = '\0';
    }
    // Другие конструкторы и разнообразные полезные функции

protected:
    size_t size_;
    char* data_;
    // Еще какие-нибудь полезные поля и методы
};
```

Я намеренно написал в теле конструктора копирования явный `for` вместо вызова `strcpy`, чтобы это посимвольное копирование не было скрытым. В отличие от `Int`, копирование `String` -- это не одна маленькая инструкция присваивания, это цикл, который для больших строк может стать очень и очень длительным. Но даже для достаточно небольших строк, скажем, символов 20, это очень затратная операция, потому что люди, пишущие прикладной код, редко хотят задумываться "а сколько же я сейчас породил временных объектов? Не стоит ли мне передать здесь эту строку по значению, а тут заменить сложение конкатенацией?". Поэтому желательно избегать ненужного копирования хотя бы автоматически.

К сожалению, как мы уже выяснили, совсем без нашей помощи компилятор обойтись не может. Но зато он умеет понимать, что такое rvalue и lvalue.

### Rvalue / lvalue
Выражения могут иметь разные категории: например, какие-то могут стоять слева от знака присваивания, а какие-то нет. Стандарт определяет разные категории так:
* A glvalue is an expression whose evaluation determines the identity of an object, bit-field, or function.
* A prvalue is an expression whose evaluation initializes an object or a bit-field, or computes the value of the operand of an operator, as specified by the context in which it appears.
* An xvalue is a glvalue that denotes an object or bit-field whose resources can be reused (usually because it is near the end of its lifetime). Example: Certain kinds of expressions involving rvalue references (8.3.2) yield xvalues, such as a call to a function whose return type is an rvalue reference or a cast to an rvalue reference type.
* An lvalue is a glvalue that is not an xvalue.
* An rvalue is a prvalue or an xvalue.

<img src="img/value_categories.png" height="200"> 
<img src="img/value_categories_2.png" height="300">

Замечательное определение и замечательная картинка, но лучше их пояснить несколько другим способом. Как можно видеть, на нижнем уровне у нас есть три категории выражений, из которых строятся другие: lvalue, xvalue и prvalue.
* lvalue *имеет адрес*, который можно будет далее использовать в программе. Например, это имена переменных (в том числе константных), элементы массива, вызов функции, возвращающей ссылку и др.
* prvalue *не имеет адреса*, к которому можно обратиться. Примерами могут выступать литералы (никогда не задумывались, чему может быть равно `&5`?), временные объекты, которые получаются при вычислении выражения и вызов функции, которая не возвращает ссылку.
* xvalue *имеет адрес, который, возможно, вот-вот перестанет быть валидным, но его (объект) еще можно использовать*. Пока что совсем не ясно даже, какие можно тут привести примеры, но в дальнейшем они появятся.

### Rvalue ссылка
Внимание, вопрос: зачем нам нужна эта информация? Дело в том, что компилятор точно знает, что какое-то значение является rvalue, поскольку он сам их и создаёт. А значит, для этих rvalue можно соорудить какой-нибудь особый конструктор, который будет вызываться вместо копирующего. Но для этого нам нужен способ отделять эти конструкторы друг от друга. Решение, которое придумали при разработке стандарта 2011 года, называется "давайте введём ссылку на rvalue!". Честно говоря, мне не нравится такое название, потому что, по крайней мере, в моей голове ссылка представляется как автоматически разыменовываемый указатель, а что такое указатель, например, на литерал, никто не знает. Но предложить более подходящее название я тоже, к сожалению, не могу, поэтому оставим такое, какое есть)

Ссылка на rvalue обозначается двумя амперсандами, типа такого:
```cpp
T&& rvalueRef = foo();
```

Соответственно, новый легковесный конструктор запишется так:
```cpp
String::String(String&& string);
```

Но в чем от него толк, как его можно использовать?

### Перемещение
А толк весьма большой. Если вы помните, ужас копирования заключается в том, что надо каждый раз создавать новый (огромный) буфер и посимвольно туда копировать предыдущую строчку. Но если мы знаем, что к нам пришел объект типа rvalue, то мы понимаем, что ему осталось жить не долго. Так зачем с ним церемониться и пытаться оставить его в таком же состоянии, тратя время на копирование? Давайте просто его обворуем!

```cpp
String::String(String&& string) :
    size_(string.size_),
    data_(string.data_) //, ...
{
    // Хоть мы его и обворовываем, оставить
    // объект в несогласованном состоянии мы не можем,
    // потому что при его смерти вызовется деструктор и
    // положит всю программу. Точно также мы не хотим
    // оставить у него его данные, потому что тогда это
    // будет просто неглубокое копирование, и когда он умрет,
    // наш буфер data_ будет очищен тоже, а мы этого не хотим!
    string.data_ = nullptr;
    string.size_ = 0;
}
```

В этом и есть смысл **перемещения** -- безнаказанно забрать все полезное к себе и не копировать, зная о скорой смерти объекта. Оно гораздо дешевле копирования, поэтому в его присутствии жизнь становится веселее. Теперь те копирования, которые мы не смогли убрать посредством copy elision, будут обращены в перемещение, что не может не радовать.

Давайте добавим в наш Int перемещающий конструктор, чтобы убедиться, что он вызывается там, где надо, и вернемся к предыдущему примеру с вложенными вызовами функций `level1`, `level2` и `level3` сразу со включенным RVO. Напомню, что код выглядел так:
```cpp
Int level1()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel1, 0);
    return valueInLevel1;
}

Int level2()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel2, level1());
    return valueInLevel2;
}

Int level3()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel3, level2());
    return valueInLevel3;
}

int main()
{
    TRACKER_DEFAULT_INITIALIZATION;
    TRACKER_ENTER;
    TRACKER_CREATE(Int, returnValueLevel1, level1());
    TRACKER_CREATE(Int, returnValueLevel2, level2());
    TRACKER_CREATE(Int, returnValueLevel3, level3());
}
```
Выхлоп трекера будет теперь таким:
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 03:16:25 2022


int main()
{
    Int level1()
    {
        <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>)
    }
    <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"returnValueLevel1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac8d0</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>)
    Int level2()
    {
        Int level1()
        {
            <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac7b0</font></b>)
        }
        <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac7b0</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac7b0</font></b>)
    }
    <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"returnValueLevel2" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac970</font></b>) from <b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>)
    Int level3()
    {
        Int level2()
        {
            Int level1()
            {
                <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac670</font></b>)
            }
            <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac7b0</font></b>) from <b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac670</font></b>)
            <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel1" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac670</font></b>)
        }
        <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>7</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>) from <b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac7b0</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel2" </font></b>(id: <b><font color=#d670d6>6</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac7b0</font></b>)
    }
    <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"returnValueLevel3" </font></b>(id: <b><font color=#d670d6>8</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4baca10</font></b>) from <b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>7</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"valueInLevel3" </font></b>(id: <b><font color=#d670d6>7</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bacab0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel3" </font></b>(id: <b><font color=#d670d6>8</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4baca10</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel2" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac970</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"returnValueLevel1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>b4bac8d0</font></b>)
}


9 objects created, 0 copied, 6 moved
</span></pre>

Ура! Зелёненькие надписи MOVE говорят нам о том, что все те красные копирования были лишними и компилятор смог заменить их на дешёвое перемещения. Но может, теперь RVO не нужно совсем? Является ли семантика перемещения обобщением copy elision?

**Нет!** RVO и семантика перемещения -- это ортогональные вещи, которые никак не пересекаются. Если отключить RVO, то последняя строчка со статистикой будет такой:
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">15 objects created, 0 copied, 12 moved</span></pre>

Конечно, 6 дополнительных перемещений это не 6 дополнительных копирований, но всё-таки 6 лишних объектов -- это как минимум 6 лишних вызовов деструктора. Глубокая мысль. В любом случае, copy elision не теряет своего значения даже в присутствии перемещающих конструкторов. Отсюда два вывода: во-первых, **разделение значений на категории позволяет разъяснить компилятору что делать с выражениями разных категорий, определяя новые перемещающие конструкторы, работающие быстрее, нежели глубокое копирование.** А во-вторых, **семантика перемещения и RVO -- это взаимно дополняющие приемы, но не заменяющие друг друга.**

## std::move
Великолепно, теперь временные объекты у нас перемещаются сами по себе, а иногда и просто не создаются. Но что, если мы сами хотим переместить куда-нибудь живой объект? Как тогда быть?

Для наглядности рассмотрим такой пример: у нас есть массив каких-нибудь объектов (пусть даже типа Int), который мы хотим сдвинуть циклически на один влево. Выглядеть это могло бы как-нибудь так:
```cpp
void rotateToLeft(std::vector<Int>& integers)
{
    if (integers.empty())
        return;
    
    Int front = integers.front();
    for (size_t idx = 1; idx < integers.size(); idx++)
        integers[idx - 1] = integers[idx];
    
    integers.back() = front;
}
```

В этом случае у нас будет выполнено `integers.size() + 1` копирований. Давайте запустим этот код и убедимся в этом (чтобы не засорять вывод, включим трекинг только на нужный нам момент).
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 11:47:57 2022


int main()
{
    void rotateToLeft(std::vector<Int>&)
    {
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>238f67b0</font></b>) from <b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>be15be40</font></b>)
        <b><font color=#f14c4c>COPY= </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>1</font></b>, addr: <b><font color=#d670d6>be15be40</font></b>) from <b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>1</font></b>, addr: <b><font color=#d670d6>be15bee0</font></b>)
        <b><font color=#f14c4c>COPY= </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>2</font></b>, addr: <b><font color=#d670d6>be15bee0</font></b>) from <b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>2</font></b>, addr: <b><font color=#d670d6>be15bf80</font></b>)
        <b><font color=#f14c4c>COPY= </font></b><b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>3</font></b>, addr: <b><font color=#d670d6>be15bf80</font></b>) from <b><font color=#f5f543>"tmp4" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>3</font></b>, addr: <b><font color=#d670d6>be15c020</font></b>)
        <b><font color=#f14c4c>COPY= </font></b><b><font color=#f5f543>"tmp4" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>4</font></b>, addr: <b><font color=#d670d6>be15c020</font></b>) from <b><font color=#f5f543>"tmp5" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>4</font></b>, addr: <b><font color=#d670d6>be15c0c0</font></b>)
        <b><font color=#f14c4c>COPY= </font></b><b><font color=#f5f543>"tmp5" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>be15c0c0</font></b>) from <b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>238f67b0</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>238f67b0</font></b>)
    }


1 object created, 6 copied, 0 moved
</span></pre>

Действительно, так и произошло. Компилятор не мог поступить по другому, потому что эти значения имели тип lvalue. Чтобы произошло перемещение, надо насильно переделать этот тип в xvalue с целью дать понять компилятору, что объект можно ограбить. У нас есть отличный инструмент для этого в лице `static_cast<T&&>`. Давайте прикастуем все копируемые объекты к rvalue-ссылке и посмотрим что получилось:
```cpp
void rotateToLeft(std::vector<Int>& integers)
{
    TRACKER_ENTER;
    if (integers.empty())
        return;

    Int front = static_cast<Int&&>(integers.front());
    for (size_t idx = 1; idx < integers.size(); idx++)
        integers[idx - 1] = static_cast<Int&&>(integers[idx]);

    integers.back() = static_cast<Int&&>(front);
}
```
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 11:53:19 2022


int main()
{
    void rotateToLeft(std::vector<Int>&)
    {
        <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>40a1f790</font></b>) from <b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>44d5de40</font></b>)
        <b><font color=#23d18b>MOVE= </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>1</font></b>, addr: <b><font color=#d670d6>44d5de40</font></b>) from <b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>1</font></b>, addr: <b><font color=#d670d6>44d5dee0</font></b>)
        <b><font color=#23d18b>MOVE= </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>2</font></b>, addr: <b><font color=#d670d6>44d5dee0</font></b>) from <b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>2</font></b>, addr: <b><font color=#d670d6>44d5df80</font></b>)
        <b><font color=#23d18b>MOVE= </font></b><b><font color=#f5f543>"tmp3" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>3</font></b>, addr: <b><font color=#d670d6>44d5df80</font></b>) from <b><font color=#f5f543>"tmp4" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>3</font></b>, addr: <b><font color=#d670d6>44d5e020</font></b>)
        <b><font color=#23d18b>MOVE= </font></b><b><font color=#f5f543>"tmp4" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>4</font></b>, addr: <b><font color=#d670d6>44d5e020</font></b>) from <b><font color=#f5f543>"tmp5" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>4</font></b>, addr: <b><font color=#d670d6>44d5e0c0</font></b>)
        <b><font color=#23d18b>MOVE= </font></b><b><font color=#f5f543>"tmp5" </font></b>(id: <b><font color=#d670d6>4</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>44d5e0c0</font></b>) from <b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>40a1f790</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp6" </font></b>(id: <b><font color=#d670d6>5</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>40a1f790</font></b>)
    }


1 object created, 0 copied, 6 moved
</span></pre>

Отлично, мы победили, можем расходиться! Или нет?

Пожалуй, что победили, но не до конца. Несмотря на то, что это решение отлично работает, для человека оно семантически непонятно. Что означает вот эта строчка `static_cast<T&&>(...)`? Зачем мы привели это что-то к двойной ссылке? Хочется, чтобы все работало, но при этом код выглядел семантически понятно:
```cpp
void rotateToLeft(std::vector<Int>& integers)
{
    if (integers.empty())
        return;

    Int front = Move(integers.front());
    for (size_t idx = 1; idx < integers.size(); idx++)
        integers[idx - 1] = Move(integers[idx]);

    integers.back() = Move(front);
}
```

Так то лучше! Давайте соорудим такую функцию (аналогичную `std::move`). Но для этого нам понадобится узнать про ссылки еще кое-что в свете работы с шаблонами.

### Шаблонная функция
Достаточно понятно, что мы не хотим писать `Move` для разных типов. Решение очевидно: функцию `Move` шаблонной! Но что она будет принимать? Ссылку? Двойную ссылку? Можно написать и то, и то. И кажется, это даже работает. Посмотрим на такой пример и на его вывод:
```cpp
template<typename T>
T&& Move(T&& obj)
{
    return static_cast<T&&>(obj);
}

template<typename T>
T&& Move(T& obj)
{
    return static_cast<T&&>(obj);
}

void testMoves()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, a, 0);
    TRACKER_CREATE(Int, b, 0);
    a = Move(b);    // b   - lvalue
    b = Move(a++);  // a++ - rvalue
}
```
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Wed Mar  9 12:11:26 2022


void testMoves()
{
    <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"a" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>09746900</font></b>)
    <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"b" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>097469a0</font></b>)
    <b><font color=#23d18b>MOVE= </font></b><b><font color=#f5f543>"a" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>09746900</font></b>) from <b><font color=#f5f543>"b" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>097469a0</font></b>)
    Int Int::operator++(int)
    {
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>097467a0</font></b>) from <b><font color=#f5f543>"a" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>09746900</font></b>)
        Int& Int::operator++()
        {
        }
        <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>09746a40</font></b>) from <b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>097467a0</font></b>)
        <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"currentValueCopy" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>097467a0</font></b>)
    }
    <b><font color=#23d18b>MOVE= </font></b><b><font color=#f5f543>"b" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>097469a0</font></b>) from <b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>09746a40</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>3</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>09746a40</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"b" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>097469a0</font></b>)
    <b><font color=#3b8eea>DTOR </font></b><b><font color=#f5f543>"a" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>09746900</font></b>)
}


4 objects created, 1 copied, 3 moved
</span></pre>

Одно копирование внутри оператора инкремента нужно и никуда оно, конечно, не делось (тем более мы и не залазили никуда внутрь нашего класса). Кажется, move работает и с rvalue, и с lvalue. На этом можно было бы разойтись. Но комитет по стандартизации языка был бы не комитетом, если бы не придумал очередную жесть.

### Правило склейки ссылок и универсальная ссылка
Дело в том, что никто не хочет писать свои шабонные функции отдельно для rvalue-ссылок и отдельно для lvalue-ссылок. Поэтому у разработчиков появился хитрый план: давайте придумаем способ писать одну функцию для всех типов ссылок, но чтобы при инстанцировании получалась именно та ссылка, которая нам нужна. Достигается это правилом склейки ссылок. Рассмотрим вот такую шаблонную функцию:
```cpp
template<typename T>
void function(T&& arg);
```

Что будет, если мы подставим сюда rvalue-ссылку на какой-то тип, например, int? Тип T в этом случае определится как `int&`, и тогда будет нужно раскрывать такое выражение:
```cpp
void function(int& && arg);
```
Что такое rvalue-ссылка на lvalue-ссылку не вполне ясно, но мы ставили своей целью при раскрытии шаблона получить вот такую сигнатуру:
```cpp
void function(int& arg);
```
Тогда введем такое правило: если при раскрытии шаблона получается rvalue-ссылка на lvalue-ссылку, мы ее _склеим_ в lvalue-ссылку. Обобщается правило так: если при раскрытии шаблона получается ссылка на ссылку и хотя бы одна из них -- lvalue-ссылка, то и итоговая ссылка получается lvalue. Иначе получим rvalue-ссылку.

Это правило позволяет делать __универсальные ссылки__ -- какое значение мы передали в шаблонную функцию, такая ссылка и получится в итоге. Благодаря такому свойству можно делать один шаблон, а не два, отсюда и название -- универсальная ссылка.

Давайте теперь сделаем один шаблон `Move`, а не два! Наверное, он будет выглядеть так:

```cpp
template<typename T>
T&& Move(T&& obj)
{
    return static_cast<T&&>(obj);
}
```

К сожалению, нет, такой вариант не прокатит :( В случае, когда `T = R&`, `Move` инстанцируется так:
```cpp
R& Move(R& obj)
{
    return static_cast<R&>(obj);
}
```

То есть функция не приводит ничего к rvalue-ссылке. Нам нужно в `static_cast` и в типе возвращаемого значения избавится от ссылки, чтобы получить просто `R`, и тогда уже все будет работать так, как мы хотим. Это делается с помощью `std::remove_reference`. `remove_reference` --- пример настоящего применения метапрограммирования. Его реалиация выглядит так:
```cpp
template <class _Ty>
struct remove_reference {
    using type                 = _Ty;
    using _Const_thru_ref_type = const _Ty;
};

template <class _Ty>
struct remove_reference<_Ty&> {
    using type                 = _Ty;
    using _Const_thru_ref_type = const _Ty&;
};

template <class _Ty>
struct remove_reference<_Ty&&> {
    using type                 = _Ty;
    using _Const_thru_ref_type = const _Ty&&;
};
```

То есть это просто шаблон, специализированный на все случаи ссылок. С его помощью мы можем получить правильно работающий `Move` на универсальных ссылках:

```cpp
template<typename T>
std::remove_reference<T>::type&& Move(T&& obj)
{
    return static_cast<std::remove_reference<T>::type&&>(obj);
}
```

Ну и разумеется, сюрприз-сюрприз: в стандартной библиотеке есть такой `Move`, и называется он, как можно догадаться, `std:move` :)

### Идеальная передача и std::forward
Осталось совсем чуть-чуть! Кажется, что теперь мы можем писать любые шаблонные функции. Но нас поджидает одна частая проблема...

Оказывается, часто универсальные ссылки используются в обертках над другими функциями (особенно в обертках над конструкторами). Например, такой функцией является `emplace_back` в `std::vector`. Это обобщение `push_back` -- функции, помещающей объект в конец динамического массива -- для произвольного количества аргументов произвольных типов (не только того типа, с которым инстанирован вектор). Давайте временно рассмотрим такую функцию для одного аргумента, чтобы не возиться с шаблонами переменного числа параметров, и представим, как она могла бы быть сделана:

```cpp
template<typename U>
void emplace_back(U&& arg)
{
    // гарантируем, что для объекта найдется место
    resize_if_necessary_(size_ + 1);

    // используем специальный вариант оператора new
    // для создания объекта на уже выделенной памяти;

    new (data_ + size_) T(arg); // <--- самое интересное место здесь!
    size_++;
}
```
Но, к сожалению, здесь есть проблема... Дело в том, что когда мы передаем `arg` в конструктор, он уже является lvalue! Это ведь именованный объект, а все именованные объекты являются lvalue. Нас интересует именно __идеальная передача__ с сохранением типа ссылки. Что делать? Обернуть в `std::move` не вариант -- так мы сменим ~~шило на мыло~~ lvalue-ссылку на rvalue-ссылку опять независимо от того, по какой ссылке мы получили объект. Вот бы узнать по какой ссылке мы получили объект...

Но ведь эта информация не потеряна! Спасение хранится в `U`! Мы можем превратить `arg` в значение исходной категории с помощью приведения типов, используя такую же склейку ссылок, как и при передаче аргумента:
```cpp
template<typename U>
void emplace_back(U&& arg)
{
    resize_if_necessary_(size_ + 1);

    new (data_ + size_) T(static_cast<U&&>(arg));
    size_++;
}
```
Как и в случае с приведением к rvalue-ссылке, когда мы превратили ее в `std:move`, для таких целей хочется заиметь обертку с понятным именем, чтобы по коду сразу было понятно, в чем заключается семантика и что вообще делает автор. В стандартной библиотеке такая функция, разумеется, имеется и называется `std::forward`. Она имеет крайне незамысловатую реализацию:
```cpp
template <typename T>
T&& forward(T& arg)
{
    return static_cast<T&&>(arg);
}
```

Эта функция принимает аргумент по обычной rvalue-ссылке (по правилу склейки, `T&` превращается в rvalue-ссылку независимо от `T`), но возвращает именно такую ссылку, которая была в `T`. Давайте протестируем ее на небольшом примере с оберткой над конструктором:
```cpp
template<typename T>
T construct_from(T&& origin)
{
    TRACKER_ENTER;
    return T(forward<T>(origin));
}

int main()
{
    TRACKER_DEFAULT_INITIALIZATION;
    TRACKER_ENTER;
    TRACKER_CREATE(Int, origin, 0);
    auto copy = construct_from(origin);
    auto move = construct_from(std::move(origin));
    TRACKER_OFF;
}
```

Получим ровно то, что хотели: в первом случае вызывается копирующий конструктор, а во втором -- перемещающий:
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Sun Mar 13 01:30:45 2022


int main()
{
    <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"origin" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>822eb980</font></b>)
    T construct_from(T&&) [with T = Int&]
    {
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>822eba20</font></b>) from <b><font color=#f5f543>"origin" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>822eb980</font></b>)
    T construct_from(T&&) [with T = Int]
    {
        <b><font color=#23d18b>MOVE </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>822ebac0</font></b>) from <b><font color=#f5f543>"origin" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>822eb980</font></b>)
    }
}


3 objects created, 1 copied, 1 moved
</span></pre>

Если бы мы не оборачивали `origin` в `forward`, то получили бы два копирования. Убедимся в этом:
<pre style = "background-color: #1e1e1e; color: #FFFFFF"><span class="inner-pre" style="font-size: 14px">
<b><font color=#f14c4c>Tracker log </font></b>generated on Sun Mar 13 01:34:10 2022


int main()
{
    <b><font color=#e5e5e5>CTOR </font></b><b><font color=#f5f543>"origin" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>8ea67210</font></b>)
    T construct_from(T&&) [with T = Int&]
    {
    }
    <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp1" </font></b>(id: <b><font color=#d670d6>1</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>8ea672b0</font></b>) from <b><font color=#f5f543>"origin" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>8ea67210</font></b>)
    T construct_from(T&&) [with T = Int]
    {
        <b><font color=#f14c4c>COPY </font></b><b><font color=#f5f543>"tmp2" </font></b>(id: <b><font color=#d670d6>2</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>8ea67350</font></b>) from <b><font color=#f5f543>"origin" </font></b>(id: <b><font color=#d670d6>0</font></b>, val: <b><font color=#d670d6>0</font></b>, addr: <b><font color=#d670d6>8ea67210</font></b>)
    }
}


3 objects created, 2 copied, 0 moved
</span></pre>

Главное не забывать явно передавать шаблонный параметр, потому что вся полезная информация лежит именно в этом параметре, и без него функция либо раскроется как для lvalue-ссылки, либо вообще не скомпилируется.

### Вывод
Описанная в этой статье _семантика перемещений_ -- это способ экономить на копировании объектов, которые вот-вот и так будут уничтожены. До стандарта С++ 11 нельзя было эффективно и одновременно красиво решать эту проблему, потому что отсутствовали какие-то языковые механизмы, позволяющие это делать. С появлением _rvalue-ссылок_ проблема создания перемещающих конструкторов и операторов присваивания наконец-то была решена, а заодно были изучены и "причесаны" категории выражений. Но одновременно с этим язык стал в очередной раз сложнее и запутаннее, а также теперь надо писать еще больше конструкторов. Для того, чтобы лишний раз не думать о премудростях разных ссылок и каких-то правилах склейки, нужны небольшие утилитарные функции и шаблоны, среди которых особенно часто используются `std::move` -- для передачи объекта по rvalue-ссылке (обычно чтобы "насильно" переместить объект), и `std::forward` для _идеальной передачи_ (чаще всего в каких-нибудь шаблонных обёртках над другими функциями).

<!-- Здесь должны быть какие нибудь ссылки и послесловие -->