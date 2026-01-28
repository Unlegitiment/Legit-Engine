#include <iostream>
#include <LITemplates/func/delegates.h>
#include <LITemplates/pointers/Auto.h>
#include <LITemplates/types/vectortypes.h>
#include <LITemplates/datastructs/dynamicarray.h>
void bufPrintf(const char* buff) {
    printf("%s", buff);
}
static void InitLegit() {
    legit::LITLogger::OutputToFunction(bufPrintf);
}
class SampleObject {
public:
    SampleObject() {
        printf("[SAMPLEOBJECT] Creating Object Default Initialization\n");
    }
    SampleObject(int operation) {
        printf("[SAMPLEOBJECT] Creating Object Integer Constructor\n");
        this->Value = operation;
    }
    SampleObject(const SampleObject& copy) {
        printf("[SAMPLEOBJECT] Copying Object using Copy Constructor\n");
        this->Value = copy.Value;
    }
    SampleObject& operator=(const SampleObject& copy) {
        printf("[SAMPLEOBJECT] Copying Object using Copy Equals\n");
        this->Value = copy.Value;
        return *this;
    }
    SampleObject(SampleObject&& move) noexcept {
        printf("[SAMPLEOBJECT] Moving Object using Move Constructor\n");
        this->Value = move.Value;
        move.Value = 0;
    }
    SampleObject& operator=(SampleObject&& move) noexcept {
        printf("[SAMPLEOBJECT] Moving object using Move Equals\n");
        this->Value = move.Value;
        move.Value = 0;
        return *this;
    }
    ~SampleObject() {
        printf("[SAMPLEOBJECT] Deleting Object\n");
    }
    bool operator==(const SampleObject& o) {
        if (o.Value == Value) return true;
        return false;
    }
    int Value{};
};
void AddRandomShit(legit::DynamicArray<SampleObject>& Buffer) {
    Buffer.EmplaceAndGrow('A');
    Buffer.EmplaceAndGrow('B');
    Buffer.EmplaceAndGrow('C');
    Buffer.EmplaceAndGrow('D');
    Buffer.EmplaceAndGrow('E');
    Buffer.EmplaceAndGrow('F');
    Buffer.EmplaceAndGrow('G');
    Buffer.EmplaceAndGrow('H');
    Buffer.EmplaceAndGrow('I');
    Buffer.EmplaceAndGrow('J');
}
legit::DynamicArray<SampleObject> Object() {
    legit::DynamicArray<SampleObject> Buffer{5};
    AddRandomShit(Buffer);
    return Buffer;
}
void PrintAll(const legit::DynamicArray<SampleObject>& Buffer) {
    for (int i = 0; i < Buffer.GetSize(); i++) {
        auto& character = Buffer.GetArray()[i];
        printf("%c", character.Value);
    }
    printf("\n");

}
int main()
{
    system("cls");
    InitLegit();
    legit::DynamicArray<SampleObject> obj = Object();
    PrintAll(obj);
    return 0;
}
