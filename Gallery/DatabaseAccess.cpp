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
	db_ = db;
	if (exists)
	{
		
	}
	
	if (!(send_query("CREATE TABLE IF NOT EXISTS USERS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL);")
		&& send_query("CREATE TABLE IF NOT EXISTS ALBUMS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, USER_ID INTEGER NOT NULL, CREATION_DATE TEXT NOT NULL, FOREIGN KEY(USER_ID) REFERENCES USERS (ID));")
		&& send_query("CREATE TABLE IF NOT EXISTS PICTURES (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, LOCATION TEXT NOT NULL,CREATION_DATE TEXT NOT NULL, ALBUM_ID INTEGER NOT NULL, FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS (ID));")
		&& send_query("CREATE TABLE IF NOT EXISTS TAGS (PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, PRIMARY KEY(PICTURE_ID, USER_ID), FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES (ID), FOREIGN KEY(USER_ID) REFERENCES USERS (ID));")))
	{
		return false;
	}
	
	
	return true;
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	std::list<Album> albums;
	sqlite3_exec(db_, "SELECT * FROM ALBUMS", callback, static_cast<void*>(&albums), nullptr);
	return albums;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::list<Album> albums;
	sqlite3_exec(db_, "SELECT * FROM ALBUMS WHERE USER_ID=" + user.getId(), callback, static_cast<void*>(&albums), nullptr);
	for (auto album : albums)
	{
		std::cout << album << std::endl;
	}
	return albums;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	send_query("INSERT INTO ALBUMS (NAME, USER_ID, CREATION_DATE) VALUES ('" + album.getName() + "', " + std::to_string(album.getOwnerId()) + ", '" + album.getCreationDate() + "');");
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	auto res = send_query("DELETE FROM PICTURES WHERE ALBUM_ID IN ( SELECT ID FROM ALBUMS WHERE NAME='" + albumName + "');");
	res = send_query("DELETE FROM ALBUMS WHERE NAME='" + albumName + "';");
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	std::list<Album> albums;
	const auto query = "SELECT * FROM ALBUMS WHERE NAME='" + albumName + "' && USER_ID=" + std::to_string(userId) + ";";
	sqlite3_exec(db_, query.c_str(), callback, &albums, nullptr);
	return false;
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
	auto albums  = getAlbums();
	for (auto album : albums)
	{
		std::cout << album << std::endl;
	}
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
	m_albums_.clear();
	m_users_.clear();
}

int DatabaseAccess::callback(void* used, int argc, char** argv, char** az_col_name)
{
	Album* a;
	auto* albums = static_cast<std::list<Album>*>(used);
	if (argv[3])
	{
		a = new Album(atoi(argv[2]), argv[1], argv[3]);
	}
	else
	{
		a = new Album(atoi(argv[2]), argv[1]);
	}
	albums->push_back(*a);
	delete a;
	
	return 0;
}

bool DatabaseAccess::send_query(const std::string& query) const
{
	char* err;
	const auto res = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &err);
	if (res != SQLITE_OK)
	{
		return false;
	}
	return true;
}
