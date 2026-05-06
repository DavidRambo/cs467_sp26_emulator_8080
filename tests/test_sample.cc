#include "test_declaration.h"
#include "../src/GraphicsDisplay.h"

TEST_CASE("Sample test") { CHECK(1); }

TEST_CASE("Object Display Value")
{
    GraphicsDisplay::GameWindow obj(10);

    CHECK(obj.ReturnValue() == 10);
}