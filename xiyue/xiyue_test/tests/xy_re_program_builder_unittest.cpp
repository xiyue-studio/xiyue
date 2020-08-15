#include "pch.h"
#include "xiyue_xy_re_program_builder.h"
#include "xiyue_xy_re_parser.h"
#include "xiyue_xy_re.h"

using namespace std;
using namespace xiyue;

#define _CHAR XyReDirective::CHAR

static XyReProgramPtr buildProgram(const wchar_t* re)
{
	XyReParser parser;
	auto ast = parser.parse(re);
	XyReProgramBuilder builder;
	XyReProgramPtr data((XyReProgram*)builder.build(ast.get()), free);
	return data;
}

static void expectInstruction(const XyReInstruction* &inst, XyReDirective d, uint8_t sd, uint8_t a1, uint8_t a2)
{
	EXPECT_EQ(inst->directive, d);
	EXPECT_EQ(inst->subDirective, sd);
	EXPECT_EQ(inst->arg1, a1);
	EXPECT_EQ(inst->arg2, a2);

	++inst;
}

static void expectInstruction(const XyReInstruction* &inst, XyReDirective d)
{
	EXPECT_EQ(inst->directive, d);
	EXPECT_EQ(inst->subDirective, 0);
	EXPECT_EQ(inst->arg1, 0);
	EXPECT_EQ(inst->arg2, 0);

	++inst;
}

static void expectInstruction(const XyReInstruction* &inst, XyReDirective d, uint8_t sd, uint16_t a)
{
	EXPECT_EQ(inst->directive, d);
	EXPECT_EQ(inst->subDirective, sd);
	EXPECT_EQ(inst->arg1, uint8_t((a & 0xff00) >> 8));
	EXPECT_EQ(inst->arg2, uint8_t(a & 0xff));

	++inst;
}

static void expectArgument(const XyReInstruction* &inst, uint32_t a)
{
	EXPECT_EQ(*reinterpret_cast<const uint32_t*>(inst), a);

	++inst;
}

TEST(XyReProgramBuilderTest, sequenceTest)
{
	auto program = buildProgram(_XR(abc));
	ASSERT_EQ(program->instructionCount, 4);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, _CHAR, 0, 'c');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, branchTest)
{
	auto program = buildProgram(_XR(a|b));
	ASSERT_EQ(program->instructionCount, 5);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, 2);
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, moreBranchTest)
{
	auto program = buildProgram(_XR(a|b|c));
	ASSERT_EQ(program->instructionCount, 8);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, 5);
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, JUMP, 0, 2);
	expectInstruction(inst, _CHAR, 0, 'c');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, starClosureTest)
{
	auto program = buildProgram(_XR(a*));
	ASSERT_EQ(program->instructionCount, 4);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, (uint16_t)-2);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, starQuestionClosureTest)
{
	auto program = buildProgram(_XR(a*?));
	ASSERT_EQ(program->instructionCount, 5);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 4, 1);
	expectInstruction(inst, DELY);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, (uint16_t)-3);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, starPlusClosureTest)
{
	auto program = buildProgram(_XR(a*+));
	ASSERT_EQ(program->instructionCount, 5);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, (uint16_t)-2);
	expectInstruction(inst, ABAN, 0, 1);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, questionClosureTest)
{
	auto program = buildProgram(_XR(a?));
	ASSERT_EQ(program->instructionCount, 3);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 1, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, questionQuestionClosureTest)
{
	auto program = buildProgram(_XR(a??));
	ASSERT_EQ(program->instructionCount, 4);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 3, 1);
	expectInstruction(inst, DELY);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, questionPlusClosureTest)
{
	auto program = buildProgram(_XR(a?+));
	ASSERT_EQ(program->instructionCount, 4);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 1, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, ABAN, 0, 1);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, plusClosureTest)
{
	auto program = buildProgram(_XR(a+));
	ASSERT_EQ(program->instructionCount, 3);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, (uint8_t)-1, 1);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, plusQuestionClosureTest)
{
	auto program = buildProgram(_XR(a+?));
	ASSERT_EQ(program->instructionCount, 5);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 3, 1);
	expectInstruction(inst, DELY);
	expectInstruction(inst, JUMP, 0, (uint16_t)-3);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, plusPlusClosureTest)
{
	auto program = buildProgram(_XR(a++));
	ASSERT_EQ(program->instructionCount, 4);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, (uint8_t)-1, 1);
	expectInstruction(inst, ABAN,0, 1);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, repeatClosureTest)
{
	auto program = buildProgram(_XR(a{3}));
	ASSERT_EQ(program->instructionCount, 4);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, rangeClosureTest)
{
	auto program = buildProgram(_XR(a{1,3}));
	ASSERT_EQ(program->instructionCount, 6);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 1, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 1, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, openRangeClosureTest)
{
	auto program = buildProgram(_XR(a{1,}));
	ASSERT_EQ(program->instructionCount, 5);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, (uint16_t)-2);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, rangeQuestionClosureTest)
{
	auto program = buildProgram(_XR(a{1,3}?));
	ASSERT_EQ(program->instructionCount, 8);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 3, 1);
	expectInstruction(inst, DELY);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 3, 1);
	expectInstruction(inst, DELY);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, openRangeQuestionClosureTest)
{
	auto program = buildProgram(_XR(a{1,}?));
	ASSERT_EQ(program->instructionCount, 6);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 4, 1);
	expectInstruction(inst, DELY);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, (uint16_t)-3);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, rangePlusClosureTest)
{
	auto program = buildProgram(_XR(a{1,3}+));
	ASSERT_EQ(program->instructionCount, 8);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 1, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, ABAN, 0, 1);
	expectInstruction(inst, SPLT, 0, 1, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, ABAN, 0, 1);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, openRangePlusClosureTest)
{
	auto program = buildProgram(_XR(a{1,}+));
	ASSERT_EQ(program->instructionCount, 6);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, (uint16_t)-2);
	expectInstruction(inst, ABAN, 0, 1);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, charClassTest)
{
	auto program = buildProgram(_XR([a-z0-9A-Z_-]));
	ASSERT_EQ(program->instructionCount, 10);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, CLSM, 0, 3, 2);
	expectArgument(inst, '0');
	expectArgument(inst, '9');
	expectArgument(inst, 'A');
	expectArgument(inst, 'Z');
	expectArgument(inst, 'a');
	expectArgument(inst, 'z');
	expectArgument(inst, '-');
	expectArgument(inst, '_');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, invCharClassTest)
{
	auto program = buildProgram(_XR([^a-z0-9A-Z_-]));
	ASSERT_EQ(program->instructionCount, 10);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, CLSM, 1, 3, 2);
	expectArgument(inst, '0');
	expectArgument(inst, '9');
	expectArgument(inst, 'A');
	expectArgument(inst, 'Z');
	expectArgument(inst, 'a');
	expectArgument(inst, 'z');
	expectArgument(inst, '-');
	expectArgument(inst, '_');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, groupTest)
{
	auto program = buildProgram(_XR((ab)c));
	EXPECT_EQ(program->numberGroupCount, 1);
	ASSERT_EQ(program->instructionCount, 6);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SVST, 0, 1);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SVED, 0, 1);
	expectInstruction(inst, _CHAR, 0, 'c');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, metaSequenceTest)
{
	auto program = buildProgram(_XR(^.\d\D\w\W\s\S\b\B\G\t\\\x20\u9527$));
	ASSERT_EQ(program->instructionCount, 17);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, HEAD);
	expectInstruction(inst, ALLM);
	expectInstruction(inst, DGIT);
	expectInstruction(inst, NDGT);
	expectInstruction(inst, XyReDirective::WORD);
	expectInstruction(inst, NWOD);
	expectInstruction(inst, SPCE);
	expectInstruction(inst, NSPC);
	expectInstruction(inst, BOND);
	expectInstruction(inst, NBND);
	expectInstruction(inst, LSPM);
	expectInstruction(inst, _CHAR, 0, '\t');
	expectInstruction(inst, _CHAR, 0, '\\');
	expectInstruction(inst, _CHAR, 0, '\x20');
	expectInstruction(inst, _CHAR, 0, 0x9527);
	expectInstruction(inst, TAIL);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, backReferenceTest)
{
	auto program = buildProgram(_XR((ab)\1));
	ASSERT_EQ(program->instructionCount, 7);
	EXPECT_EQ(program->numberGroupCount, 1);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SVST, 0, 1);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SVED, 0, 1);
	expectInstruction(inst, BREF, 0, 1);
	expectInstruction(inst, WBRF);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, lookBehindTest)
{
	auto program = buildProgram(_XR(a(?=ab)));
	ASSERT_EQ(program->instructionCount, 7);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, CALL, 0, 2, 5);
	expectArgument(inst, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SUCC);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, negLookBehindTest)
{
	auto program = buildProgram(_XR(a(?!ab)));
	ASSERT_EQ(program->instructionCount, 7);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, CALL, 1, 2, 5);
	expectArgument(inst, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SUCC);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, fixedLookAheadTest)
{
	auto program = buildProgram(_XR(a(?<=ab)));
	ASSERT_EQ(program->instructionCount, 7);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, CALL, 2, 2, 5);
	expectArgument(inst, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SUCC);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, fixedNegLookAheadTest)
{
	auto program = buildProgram(_XR(a(?<!ab)));
	ASSERT_EQ(program->instructionCount, 7);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, CALL, 3, 2, 5);
	expectArgument(inst, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SUCC);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, flexLookAheadTest)
{
	auto program = buildProgram(_XR(a(?<=a*)));
	ASSERT_EQ(program->instructionCount, 9);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, CALL, 4, 3, 7);
	expectArgument(inst, 0);
	expectArgument(inst, UINT32_MAX);
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, (uint16_t)-2);
	expectInstruction(inst, SUCC);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, flexNegLookAheadTest)
{
	auto program = buildProgram(_XR(a(?<!a*)));
	ASSERT_EQ(program->instructionCount, 9);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, CALL, 5, 3, 7);
	expectArgument(inst, 0);
	expectArgument(inst, UINT32_MAX);
	expectInstruction(inst, SPLT, 0, 1, 3);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, JUMP, 0, (uint16_t)-2);
	expectInstruction(inst, SUCC);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, namedGroupTest)
{
	auto program = buildProgram(_XR((?<name>ab)\k<name>));
	ASSERT_EQ(program->instructionCount, 7);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SVST, 0, (uint16_t)-1);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SVED, 0, (uint16_t)-1);
	expectInstruction(inst, BREF, 0, (uint16_t)-1);
	expectInstruction(inst, WBRF);
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, noCaptureGroupTest)
{
	auto program = buildProgram(_XR((?:ab)c));
	ASSERT_EQ(program->instructionCount, 4);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, _CHAR, 0, 'c');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, fixedGroupTest)
{
	auto program = buildProgram(_XR((?>a?)b));
	ASSERT_EQ(program->instructionCount, 5);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SPLT, 0, 1, 2);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, ABAN, 0, 1);
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, switchTest)
{
	auto program = buildProgram(_XR((?i)ab(?-i)c));
	ASSERT_EQ(program->instructionCount, 6);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SWCH, 0, 'i', 0);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SWCH, 1, 'i', 0);
	expectInstruction(inst, _CHAR, 0, 'c');
	expectInstruction(inst, SUCC);
}

TEST(XyReProgramBuilderTest, embeddedSwitchTest)
{
	auto program = buildProgram(_XR((?i:ab)c));
	ASSERT_EQ(program->instructionCount, 6);
	const XyReInstruction* inst = program->instructions();
	expectInstruction(inst, SWCH, 0, 'i', 0);
	expectInstruction(inst, _CHAR, 0, 'a');
	expectInstruction(inst, _CHAR, 0, 'b');
	expectInstruction(inst, SWCH, 1, 'i', 0);
	expectInstruction(inst, _CHAR, 0, 'c');
	expectInstruction(inst, SUCC);
}
