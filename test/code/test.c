/*
** Здесь можно наблюдать пример
** структуры плагинов для сервера,
** использовать только для ознакомления
** с PluginAPI.
*/

/*
** Хедеры сервера. Файл core.h должен быть
** всегда подключен в главном файле кода плагина.
*/
#include <core.h>
#include <event.h>
#include <command.h>
#include <str.h>

// Хедеры плагина
#include "test.h"

/*
** В функцию эвента передаётся первый параметр,
** который был передан функции вызова эвента,
** если параметр был один, то передаётся только он,
** без поинтера на пачало параметров функции.
** Мне показалось удобным кастование аргументов
** в структуру для дальнейшего взаимодействия с ними
** более удобного способа я не придумал.
*/
static void onmesgfunc(void* param) {
  if(enabled)
    *((onMessage_p)param)->type = CPE_ANNOUNCE;
}

/*
** При вызове команды из консоли сервера аргумент "caller" будет NULL.
** Также стоит заметить, что "args" тоже будет NULL при отсутствии аргументов.
*/
static bool CHandler_Plugtest(const char* args, Client caller, char* out) {
	(void)args; (void)caller;
  String_Copy(out, MAX_CMD_OUT, "This command registred by testplugin." DLIB_EXT);
  return true;
}

static bool CHandler_Atoggle(const char* args, Client caller, char* out) {
	// Макрос проверяет была ли запущена команда администратором
	Command_OnlyForOP;
	(void)args;

  enabled ^= 1;
	String_FormatBuf(out, MAX_CMD_OUT, "Announce chat %s", MODE(enabled));
  return true;
}

/*
** Исходя из названия команды,
** даже самому тупенькому будет понятно:
** Эта команда уничтожает сама себя после
** исполнения, то есть, когда она будет
** вызвана однажды - её нельзя будет вызвать
** вновь, вплоть до перезапуска сервера.
*/
static bool CHandler_SelfDestroy(const char* args, Client caller, char* out) {
	(void)args; (void)caller;
	Command_Unregister("selfdestroy");
	String_Copy(out, MAX_CMD_OUT, "This command can't be called anymore");
	return true;
}

/*
** Если в начало обработчика команды
** сунуть макрос Command_OnlyForClient,
** то команда будет выполнена только в том
** случае, если вызвана она была игроком,
** в противном случае в консоль выводится
** сообщение о том, что команду может вызвать
** только игрок.
*/
static bool CHandler_ClientOnly(const char* args, Client caller, char* out) {
	Command_OnlyForClient;
	(void)args;

	String_FormatBuf(out, MAX_CMD_OUT, "Client-only command called by %s", Client_GetName(caller));
	return true;
}

EXP int32_t Plugin_ApiVer = CPLUGIN_API_NUM; // Текущая версия API плагинов 1.
EXP bool Plugin_Load(void) { // Основная функция, вызывается после подгрузки плагина.
  Event_RegisterVoid(EVT_ONMESSAGE, onmesgfunc); // Регистрация обработчика эвента.
  Command_Register("plugtest", CHandler_Plugtest);
  Command_Register("atoggle", CHandler_Atoggle);
	Command_Register("selfdestroy", CHandler_SelfDestroy);
	Command_Register("clonly", CHandler_ClientOnly);
  Log_Info("Test plugin loaded"); // Отправка в консоль INFO сообщения.
  Log_Debug("It's a debug message");
  Log_Warn("It's a warning message");

	/*
	** Если функция вернула true, значит
	** плагин удалось успешно загрузить.
	** Если функция вернёт false, сервер
	** выгрузит динамическую библиотеку
	** плагина из памяти и не будет
	** больше на неё ссылаться.
	*/
  return true;
}
EXP bool Plugin_Unload(void) {
	Event_Unregister(EVT_ONMESSAGE, (void*)onmesgfunc);
	Command_Unregister("plugtest");
	Command_Unregister("atoggle");
	Command_Unregister("selfdestroy");
	Command_Unregister("clonly");

	/*
	** Возврат true говорит о том, что
	** плагин может быть выгружен в данный
	** момент без ущерба работоспособности
	** сервера. Если функция вернёт false
	** то плагин останется в памяти
	** нетронутым.
	*/
	return true;
}
