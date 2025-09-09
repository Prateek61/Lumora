#include "Lumora.h"

struct Test
{
	std::vector<std::string> Stuff;
	int Value;
	std::filesystem::path Path;
};
VISITABLE_STRUCT(Test, Stuff, Value, Path);
LM_REGISTER_FOR_SERIALIZATION(Test);

const char* LUA_SCRIPT = R"(
return {
	Stuff = { "Hello", "World" },
	Value = 42,
	Path = "C:/Some/Path"
}
)";

int main()
{
	Lumora::Log::Init();

	Lumora::LuaSerializer serializer;

	Test t1 = { {"Hello", "I'm", "Under"}, 45, "The Water" };
	auto t1_script = serializer.SerializeToLuaScript(t1);
	LM_TRACE("Serialized:\n{0}", t1_script);

	// Runtime example
	Lumora::Ref<void> obj = serializer.DeserializeFromLuaScript("Test", LUA_SCRIPT);
	Lumora::Ref<Test> t2 = Lumora::StaticRefCast<Test>(obj);
	LM_TRACE("Deserialized:\nStuff = [{}, {}], Value = {}, Path = {}", t2->Stuff[0], t2->Stuff[1], t2->Value, t2->Path.string());
}