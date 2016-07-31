#include <common/common.hpp>
#include <common/database.hpp>
#include <common/serializable.hpp>

class FeDbSerializable : public FeSerializable
{
public:
#define props(_d)							\
	_d(uint32,		ID)						\

	virtual const char* GetSecondaryKey() { return ""; }

	FeDbSerializable()
	{
		ID = FE_INVALID_ID;
	}

	bool HasSecondaryKey() { return strlen(GetSecondaryKey()) > 0; }

	FE_DECLARE_CLASS_BODY(props, FeDbSerializable, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDbSerializable)
// ------------------------------------------------------------------------------------------
class FeDbSerializableNamed : public FeDbSerializable
{
public:
	#define props(_d)							\
	_d(FeString,	Name)						\

	FeDbSerializableNamed() : FeDbSerializable()
	{
	}

	virtual const char* GetSecondaryKey() override { return "Name"; }

	FE_DECLARE_CLASS_BODY(props, FeDbSerializableNamed, FeDbSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDbSerializableNamed)
// ------------------------------------------------------------------------------------------
class FeDataGameDump : public FeDbSerializable
{
public:
	#define props(_d)							\
	_d(uint32,	Checksum)						\
	_d(FeString,Name)							\
	_d(uint32,	CountryCode)					\
	_d(uint32,	Flags)							\

	FeDataGameDump() : FeDbSerializable()
	{}

	virtual const char* GetSecondaryKey() override { return "Checksum"; }

	FE_DECLARE_CLASS_BODY(props, FeDataGameDump, FeDbSerializable)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameDump)
// ------------------------------------------------------------------------------------------
class FeDataGameData : public FeDbSerializable
{
public:
	static const uint32 MaxDumpsCount = 10;

	#define props(_d)							\
	_d(FeTPtr<FeDataGameDump>,	Dump0)			\
	_d(FeTPtr<FeDataGameDump>,	Dump1)			\
	_d(FeTPtr<FeDataGameDump>,	Dump2)			\
	_d(FeTPtr<FeDataGameDump>,	Dump3)			\
	_d(FeTPtr<FeDataGameDump>,	Dump4)			\
	_d(FeTPtr<FeDataGameDump>,	Dump5)			\
	_d(FeTPtr<FeDataGameDump>,	Dump6)			\
	_d(FeTPtr<FeDataGameDump>,	Dump7)			\
	_d(FeTPtr<FeDataGameDump>,	Dump8)			\
	_d(FeTPtr<FeDataGameDump>,	Dump9)			\

	FeDataGameData() : FeDbSerializable()
	{}

	FeTPtr<FeDataGameDump>& GetDump(uint32 iDumpIdx)
	{
		switch (iDumpIdx)
		{
			case 0:		return GetDump0();
			case 1:		return GetDump1();
			case 2:		return GetDump2();
			case 3:		return GetDump3();
			case 4:		return GetDump4();
			case 5:		return GetDump5();
			case 6:		return GetDump6();
			case 7:		return GetDump7();
			case 8:		return GetDump8();
			case 9:		return GetDump9();
			default:
			{
				FE_ASSERT(false, "invalid game dump index !");
				return GetDump0();
			}
		}
	}

	FE_DECLARE_CLASS_BODY(props, FeDataGameData, FeDbSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameData)
// ------------------------------------------------------------------------------------------
class FeDataPlatformArts : public FeDbSerializable
{
public:
	#define props(_d)

	FeDataPlatformArts() : FeDbSerializable()
	{}

	FE_DECLARE_CLASS_BODY(props, FeDataPlatformArts, FeDbSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataPlatformArts)
// ------------------------------------------------------------------------------------------
class FeDataGameGenre : public FeDbSerializableNamed
{
public:
	#define props(_d)		\
	_d(FeString, Overview)	\

	FeDataGameGenre() : FeDbSerializableNamed()
	{}

	FE_DECLARE_CLASS_BODY(props, FeDataGameGenre, FeDbSerializableNamed)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameGenre)
// ------------------------------------------------------------------------------------------
class FeDataGameArts : public FeDbSerializable
{
public:
	#define props(_d)

	FeDataGameArts() : FeDbSerializable()
	{}

	FE_DECLARE_CLASS_BODY(props, FeDataGameArts, FeDbSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameArts)
// ------------------------------------------------------------------------------------------
class FeDataGamePublisher : public FeDbSerializableNamed
{
public:
	#define props(_d)

	FeDataGamePublisher() : FeDbSerializableNamed()
	{}

	FE_DECLARE_CLASS_BODY(props, FeDataGamePublisher, FeDbSerializableNamed)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGamePublisher)
// ------------------------------------------------------------------------------------------
class FeDataGameDeveloper : public FeDbSerializableNamed
{
public:
	#define props(_d)

	FeDataGameDeveloper() : FeDbSerializableNamed()
	{}

	FE_DECLARE_CLASS_BODY(props, FeDataGameDeveloper, FeDbSerializableNamed)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameDeveloper)
// ------------------------------------------------------------------------------------------
class FeDataPlatform : public FeDbSerializableNamed
{
public:
	#define props(_d)							\
	_d(FeString,					Overview)	\
	_d(FeTPtr<FeDataPlatformArts>,	Arts)		\

	FeDataPlatform() : FeDbSerializableNamed()
	{}

	FE_DECLARE_CLASS_BODY(props, FeDataPlatform, FeDbSerializableNamed)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataPlatform)
// ------------------------------------------------------------------------------------------
class FeDataGame : public FeDbSerializableNamed
{
public:
	#define props(_d)											\
	_d(FeTPtr<FeDataGameData>,		Data)						\
	_d(FeTPtr<FeDataPlatform>,		Platform)					\
	_d(FeTPtr<FeDataGameGenre>,		Genre)						\
	_d(FeTPtr<FeDataGameArts>,		Arts)						\
	_d(FeTPtr<FeDataGamePublisher>,	Publisher)					\
	_d(FeTPtr<FeDataGameDeveloper>,	Developer)					\
	_d(FeString,					ReleaseDate)				\
	_d(FeString,					Overview)					\
	_d(bool,						Adult)						\
	_d(uint32,						PlayersCount)				\
	_d(uint32,						PlayersCountSimultaneous)	\
	_d(bool,						IsScrapped)					\

	FeDataGame() : FeDbSerializableNamed()
	{
		Adult = false;
		PlayersCount = 1;
		PlayersCountSimultaneous = 1;
		IsScrapped = false;
	}

	FE_DECLARE_CLASS_BODY(props, FeDataGame, FeDbSerializableNamed)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGame)

// ------------------------------------------------------------------------------------------
//class FeDataFile : public FeSerializable
//{
//public:
//	#define props(_d)							\
//	_d(FeNTArray<FeDataGame>,		Games)		\
//
//	FE_DECLARE_CLASS_BODY(props, FeDataFile, FeSerializable)
//	#undef props
//};
//FE_DECLARE_CLASS_BOTTOM(FeDataFile)