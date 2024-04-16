/* ------------------------------------------------------------------------------------/
 /     Модуль для работы с кнопоками на микроконтроллере stm32f4 (и не только).
 * ------------------------------------------------------------------------------------/
 *
 *  Автор: VadRov
 *  Copyright (C) 2021, VadRov, all right reserved.
 *
 *  Допускается свободное распространение.
 *  При любом способе распространения указание автора ОБЯЗАТЕЛЬНО.
 *  В случае внесения изменений и распространения модификаций указание первоначального автора ОБЯЗАТЕЛЬНО.
 *  Распространяется по типу "как есть", то есть использование осуществляется на свой страх и риск.
 *  Автор не предоставляет никаких гарантий.
 *
 *  Возможности модуля:
 *  - Поддержка до 16 кнопок, с учетом того, что на 1 кнопку выделяется 1 вывод микроконтроллера,
 *  настроенный на вход с подтяжкой по питанию.
 *  - Доступна настройка фильтра устранения дребезга.
 *  - Доступен буфер состояний кнопок с настраиваемой глубиной.
 *  - Доступна настройка периодов задержки до первого автоповтора нажатия кнопок и последующих автоповторов.
 *  - Доступно добавление кнопок для опроса посредством вызова соответствующей функции.
 *  - Доступна установка статусов кнопки: "активна" - участвует в опросе, "пассивна" - не участвует в опросе.
 *  - Доступен опрос статуса кнопки (активна либо пассивна).
 *
 *  Выводы микроконтроллера, к которым подключаются кнопки должны быть настроены, как входы с подтяжкой по питанию. Как вариант, с использованием
 *	внутренней подтяжки микроконтроллера pull-up, либо внешней через подтягивающий резистор, например, номиналом 10 кОм.
 *	При нажатии кнопки должна осуществляется притяжка входа к "земле" (GND).
 *
 *	Для работы библиотеки требуется 1 таймер с настроенным прерыванием по обновлению.
 *	В функцию обработки прерывания таймера (например, в файле stm32f4xx_it.c) необходимо включить cтроку:
 *	KEYB_Input_Keys(); //вызов процедуры опроса кнопок
 *	В файле вызова KEYB_Input_Keys() необходимо определить прототип функции: extern void KEYB_Input_Keys(void)
 *	или подключить заголовочный файл keyboard.h: #include "keyboard.h"
 *	Да, и не забудьте в своей программе разрешить прерывания по обновлению таймера и включить таймер.
 *	Например, вот так для stm32f4:
 *	KEYB_TIM->DIER |= TIM_DIER_UIE;
	KEYB_TIM->CR1  |= TIM_CR1_CEN;
 *  где KEYB_TIM - номер таймера, определяемый в файле keyboard.h
 *
 *	Свои кнопки для опроса добавляем с использованием функции
 *	KEYB_Add_Button(key_port, key_pin, key_bit, key_status);
 *	где:  key_port   - указатель на используемый порт кнопки, например, GPIOA (порт А);
 *		  key_pin    - используемый пин порта кнопки, например, LL_GPIO_PIN_3 (3 пин);
 *		  key_bit    - номер бита, соответствующий кнопке в переменной состояний кнопок, например, 0 (0 бит)
 *		  key_status - статус кнопки: активная (KEYB_BUTTON_ACTIVE) либо пассивная (KEYB_BUTTON_PASSIVE),
 *		               активная опрашивается, а пассивная - нет.
 *
 *  Текущее состояние кнопок (из буфера) получаем путем вызова функции KEYB_Inkeys(), которая в 32-разрядной переменной
 *  возвращает, т.н., "битовое поле", в котором каждой кнопке соответствует бит, номер которого является номером бита
 *  заданным для кнопки в "Описании кнопок". Установленный бит - кнопка нажата, сброшенный - не нажата.
 *
 *  Статус кнопок (активная/пассивная) можно менять путем вызова функции KEYB_Set_Button_Status(key_bit, key_status),
 *  передав номер бита кнопки из описания и желаемый статус.
 *
 *  Статус кнопки (активная/пассивная) можно получить путем вызова функции KEYB_Get_Button_Status(key_bit),
 *  передав номер бита кнопки из описания.
 *
 *  Модуль легко редактируется для использования с любым микроконтроллером (не только stm32).
 *
 *  Все настройки модуля определены в файле keyboard.h
 *
 *  https://www.youtube.com/@VadRov
 *  https://dzen.ru/vadrov
 *  https://vk.com/vadrov
 *  https://t.me/vadrov_channel
 *
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "main.h"

/*---------------------------------------------------------------------------------------------------------------
/                           Настраиваемые параметры модуля
-----------------------------------------------------------------------------------------------------------------*/

#define NOSTANDART_USE	1

#if NOSTANDART_USE == 0
//Таймер
#define KEYB_TIM		TIM10
#else
extern uint16_t KEYB_key_buff[];
extern uint8_t  KEYB_count_key;
extern uint8_t  KEYB_all_button;
#endif
//Описание кнопок. Вариант. Каждой кнопке соответствует определенный бит.
//Библиотека допускает работу с 32 кнопками (по числу бит в 32-разрядном числе)
#define KEYB_UP			0	//0 бит - кнопка вверх
#define KEYB_DOWN		1	//1 бит - кнопка вниз
#define KEYB_LEFT		2	//2 бит - кнопка влево
#define KEYB_RIGHT		3	//3 бит - кнопка вправо
#define KEYB_F1   		4	//4 бит
#define KEYB_F2   		5	//4 бит
//......................6   //6 бит
//и так далее до 15 бита
#if NOSTANDART_USE == 0
//Вариант настройки ввода (приведены параметры для таймера, настроенного на 200 Гц)
#define KEYB_CONTACT_PER  2 //Период для устранения дребезга контактов. Зависит от частоты таймера опроса и характеристик кнопки.
							//Это время, за которое состояние кнопки считается установившимся, т.е. отсутствуют переходные процессы,
							//т.н., дребезг контактов, и кнопка из одного устойчивого состояния перешла в другое устойчивое состояние.
							//Определяется опытным путем, в т.ч., с использованием логического анализатора либо осциллографа.
							//Задается в количестве периодов таймера опроса.
#define KEYB_REPPRE_PER	100 //Период до первого автоповтора
							//Задается в количестве периодов таймера опроса.
#define KEYB_REPEAT_PER	  8 //Период последующих автоповторов при удержании кнопки.
							//Задается в количестве периодов таймера опроса.
#endif

#define KEYB_SIZE_BUFFER 20 //Размер буфера ввода. Определяет размер буфера (в глубину) для хранения предыдущих состояний кнопок.
/* -----------------------------------------------------------------------------
 *                            Конец настроек модуля
 *------------------------------------------------------------------------------*/

#if NOSTANDART_USE == 0
//Структура, описывающая кнопку
//key_port - указатель на порт, к которому подключена кнопка
//key_pin - номер пина, к которому подключена кнопка
//key_bit - номер бита, определяющий кнопку в переменной состояний кнопок.
//key_status - флаг, определяющий активна кнопка (KEYB_BUTTON_ACTIVE) или нет (KEYB_BUTTON_PASSIVE).

typedef enum {
	KEYB_BUTTON_ACTIVE = 0,
	KEYB_BUTTON_PASSIVE,
	KEYB_BUTTON_UNKNOW
} KEYB_BUTTON_STATUS;

typedef struct {
	GPIO_TypeDef *key_port;
	uint32_t key_pin;
	uint8_t key_bit;
	KEYB_BUTTON_STATUS key_status;
} KEYB_key_map_;

//Функция добавляет кнопку для опроса
//на входе порт, пин, бит, статус
void KEYB_Add_Button(GPIO_TypeDef *key_port, uint32_t key_pin, uint8_t key_bit, KEYB_BUTTON_STATUS key_status);

//Функция возвращает статус кнопки
KEYB_BUTTON_STATUS KEYB_Get_Button_Status(uint8_t key_bit);

//Функция устанавливает статус кнопки
void KEYB_Set_Button_Status(uint8_t key_bit, KEYB_BUTTON_STATUS key_status);

//Функция опрашивает кнопки в прерывании таймера.
//Вызов этой функции необходимо добавить в обработчик прерывания таймера.
void KEYB_Input_Keys(void);

#endif

//Функция возвращает битовое поле клавиш из текушей позиции буфера ввода.
//Используется в пользовательской программе для реализации опроса кнопок.
//Для того, чтобы узнать нажата кнопка или нет, необходимо опросить соответствующий
//ей бит в возвращенном значении. Этот бит задается в определениях кнопок (см. Описание кнопок выше).
//Кнопки для опроса добавляются функцией KEYB_Add_Button.
uint16_t KEYB_Inkeys(void);

//Функция возвращает 1, если нажата какая-либо кнопка. В противном случае возвращает 0.
//Состояние кнопок не удаляется из входного буфера
uint8_t KEYB_kbhit(void);

#endif /* KEYBOARD_H_ */