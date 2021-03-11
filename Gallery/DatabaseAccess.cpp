#include "DatabaseAccess.h"
#include "io.h"


bool DatabaseAccess::open()
{
	sqlite3* db;
	const std::string dbFileName = "MyDB.sqlite";

	const auto exists = _access(dbFileName.c_str(), 0);
	const auto res = sqlite3_open(dbFileName.c_str(), &db);

	if (res != SQLITE_OK)
	{
		db = nullptr;
		std::cerr << "Failed to open db" << std::endl;
		return false;
	}

	if (exists)
	{
		if (!(send_query("CREATE TABLE USERS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL);")
			&& send_query("CREATE TABLE ALBUMS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, USER_ID INTEGER NOT NULL, CREATION_DATE TEXT NOT NULL, FOREIGN KEY(USER_ID) REFERENCES USERS (ID));")
			&& send_query("CREATE TABLE PICTURES (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, LOCATION TEXT NOT NULL,CREATION_DATE TEXT NOT NULL, ALBUM_ID INTEGER NOT NULL, FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS (ID));")
			&& send_query("CREATE TABLE TAGS (PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, PRIMARY KEY(PICTURE_ID, USER_ID), FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES (ID), FOREIGN KEY(USER_ID) REFERENCES USERS (ID));")))
		{
			return false;
		}
	}

	
	db_ = db;
	return true;
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	return std::list<Album>();
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	return std::list<Album>();
}

void DatabaseAccess::createAlbum(const Album& album)
{
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	return true;
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	return Album(1, "a");
}

void DatabaseAccess::closeAlbum(Album& pAlbum)
{
}

void DatabaseAccess::printAlbums()
{
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
}

void DatabaseAccess::printUsers()
{
}

void DatabaseAccess::createUser(User& user)
{
}

void DatabaseAccess::deleteUser(const User& user)
{
}

bool DatabaseAccess::doesUserExists(int userId)
{
	return true;
}

User DatabaseAccess::getUser(int userId)
{
	return User(1, "a");
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	return 0;
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	return 0;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	return 0;
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	return 0;
}

User DatabaseAccess::getTopTaggedUser()
{
	return User(1, "a");
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	return Picture(1, "a");
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	return std::list<Picture>();
}

void DatabaseAccess::close()
{
	sqlite3_close(db_);
	db_ = nullptr;
}

void DatabaseAccess::clear()
{
	char** err = nullptr;
	auto res = sqlite3_exec(db_, "DELETE from ALBUMS", nullptr, nullptr, err);
	if (res != SQLITE_OK)
	{
		
	}
}

bool DatabaseAccess::send_query(const std::string& query) const
{
	char** err = nullptr;
	const auto res = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, err);
	if (res != SQLITE_OK)
	{
		std::cerr << err << std::endl;
		return false;
	}
	return true;
}
