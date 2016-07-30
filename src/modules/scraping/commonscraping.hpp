#include <common/common.hpp>
#include <common/database.hpp>
#include <common/serializable.hpp>

class FeDataGameData : public FeSerializable
{
public:
	#define props(_d)							\
	_d(uint32,		ID)							\

	FeDataGameData()
	{
	}

	FE_DECLARE_CLASS_BODY(props, FeDataGameData, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameData)
// ------------------------------------------------------------------------------------------
class FeDataPlatformArts : public FeSerializable
{
public:
	#define props(_d)							\
	_d(uint32,		ID)							\

	FeDataPlatformArts()
	{
	}

	FE_DECLARE_CLASS_BODY(props, FeDataPlatformArts, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataPlatformArts)
// ------------------------------------------------------------------------------------------
class FeDataGameGenre : public FeSerializable
{
public:
	#define props(_d)							\
	_d(uint32,		ID)							\
	_d(FeString,	Name)						\

	FeDataGameGenre()
	{
	}

	FE_DECLARE_CLASS_BODY(props, FeDataGameGenre, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameGenre)
// ------------------------------------------------------------------------------------------
class FeDataGameArts : public FeSerializable
{
public:
	#define props(_d)							\
	_d(uint32,		ID)							\

	FeDataGameArts()
	{
	}

	FE_DECLARE_CLASS_BODY(props, FeDataGameArts, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameArts)
// ------------------------------------------------------------------------------------------
class FeDataGamePublisher : public FeSerializable
{
public:
	#define props(_d)							\
	_d(uint32,		ID)							\
	_d(FeString,	Name)						\

	FeDataGamePublisher()
	{
	}

	FE_DECLARE_CLASS_BODY(props, FeDataGamePublisher, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGamePublisher)
// ------------------------------------------------------------------------------------------
class FeDataGameDeveloper : public FeSerializable
{
public:
	#define props(_d)							\
	_d(uint32,		ID)							\
	_d(FeString,	Name)						\

	FeDataGameDeveloper()
	{
	}

	FE_DECLARE_CLASS_BODY(props, FeDataGameDeveloper, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGameDeveloper)
// ------------------------------------------------------------------------------------------
class FeDataPlatform : public FeSerializable
{
public:
	#define props(_d)							\
	_d(uint32,						ID)			\
	_d(FeString,					Name)		\
	_d(FeString,					Overview)	\
	_d(FeTPtr<FeDataPlatformArts>,	Arts)		\

	FeDataPlatform()
	{
	}

	FE_DECLARE_CLASS_BODY(props, FeDataPlatform, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataPlatform)
// ------------------------------------------------------------------------------------------
class FeDataGame : public FeSerializable
{
public:
	#define props(_d)											\
	_d(uint32,						ID)							\
	_d(FeString,					Title)						\
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

	FeDataGame()
	{
		ID = 0;
		Adult = false;
		PlayersCount = 1;
		PlayersCountSimultaneous = 1;
		IsScrapped = false;
	}

	FE_DECLARE_CLASS_BODY(props, FeDataGame, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataGame)
// ------------------------------------------------------------------------------------------
class FeDataFile : public FeSerializable
{
public:
	#define props(_d)							\
	_d(FeNTArray<FeDataGame>,		Games)		\

	FE_DECLARE_CLASS_BODY(props, FeDataFile, FeSerializable)
	#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeDataFile)