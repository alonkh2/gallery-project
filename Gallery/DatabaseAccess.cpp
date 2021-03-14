#include "DatabaseAccess.h"
#include "io.h"
#include "ItemNotFoundException.h"
#include "MyException.h"


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
	
	if (!(send_query("CREATE TABLE IF NOT EXISTS USERS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL);")
		&& send_query("CREATE TABLE IF NOT EXISTS ALBUMS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, USER_ID INTEGER NOT NULL, CREATION_DATE TEXT NOT NULL, FOREIGN KEY(USER_ID) REFERENCES USERS (ID));")
		&& send_query("CREATE TABLE IF NOT EXISTS PICTURES (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, LOCATION TEXT NOT NULL, CREATION_DATE TEXT NOT NULL, ALBUM_ID INTEGER NOT NULL, FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS (ID));")
		&& send_query("CREATE TABLE IF NOT EXISTS TAGS (PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, PRIMARY KEY(PICTURE_ID, USER_ID), FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES (ID), FOREIGN KEY(USER_ID) REFERENCES USERS (ID));")))
	{
		return false;
	}
	
	
	return true;
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	std::list<Album> albums;
	sqlite3_exec(db_, "SELECT * FROM ALBUMS", a_callback, static_cast<void*>(&albums), nullptr);
	for (auto& album : albums)
	{
		const auto id = get_album_id(album.getName());
		for (const auto& pic : get_pictures(id))
		{
			album.addPicture(pic);
		}
	}
	return albums;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::list<Album> albums = getAlbums();
	std::list<Album> user_albums;
	/*const auto query = "SELECT * FROM ALBUMS WHERE USER_ID=" + std::to_string(user.getId());
	sqlite3_exec(db_, query.c_str(), a_callback, static_cast<void*>(&albums), nullptr);*/
	for (const auto& album : albums)
	{
		if (album.getOwnerId() == user.getId())
		{
			user_albums.push_back(album);
		}
	}
	return user_albums;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	if (!doesUserExists(album.getOwnerId()))
	{
		throw MyException("No user with id " + std::to_string(album.getOwnerId()) + " exists");
	}
	
	send_query("INSERT INTO ALBUMS (NAME, USER_ID, CREATION_DATE) VALUES ('" + album.getName() + "', " + std::to_string(album.getOwnerId()) + ", '" + album.getCreationDate() + "');");
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	auto res = send_query("DELETE FROM TAGS WHERE PICTURE_ID IN (SELECT PICTURES.ID FROM PICTURES INNER JOIN ALBUMS ON PICTURES.ALBUM_ID = ALBUMS.ID WHERE ALBUMS.NAME='" + albumName + "');");
	res = send_query("DELETE FROM PICTURES WHERE ALBUM_ID IN (SELECT ID FROM ALBUMS WHERE NAME='" + albumName + "');");
	res = send_query("DELETE FROM ALBUMS WHERE NAME='" + albumName + "';");
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	std::list<Album> albums;
	const auto query = "SELECT * FROM ALBUMS WHERE NAME='" + albumName + "' AND USER_ID=" + std::to_string(userId) + ";";
	sqlite3_exec(db_, query.c_str(), a_callback, &albums, nullptr);
	
	if (albums.empty())
	{
		return false;
	}
	albums.clear();
	return true;
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	for (auto album : getAlbums())
	{
		if (album.getName() == albumName)
		{
			return album;
		}
	}
	throw MyException("No albums with name " + albumName + " exists");
}

void DatabaseAccess::closeAlbum(Album& pAlbum)
{
	
}

void DatabaseAccess::printAlbums()
{
	auto albums  = getAlbums();
	if (albums.empty())
	{
		throw MyException("There are no existing albums.");
	}
	for (const auto& album : albums)
	{
		std::cout << album << std::endl;
	}
	albums.clear();
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, Picture& picture)
{
	auto id = get_album_id(albumName);
	if (id == -1)
	{
		throw MyException("No album with name " + albumName + " exists");
	}
	
	send_query("INSERT INTO PICTURES (NAME, LOCATION, CREATION_DATE, ALBUM_ID) VALUES ('" + picture.getName() + "', '" + picture.getPath() + "', '" + picture.getCreationDate() + "', " + std::to_string(id) + ");");
	const auto query = "SELECT * FROM PICTURES WHERE NAME='" + picture.getName() + "' ORDER BY ID DESC LIMIT 1;";
	sqlite3_exec(db_, query.c_str(), id_callback, &id, nullptr);
	picture.setId(id);
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	const auto id = get_album_id(albumName);
	send_query("REMOVE FROM PICTURES WHERE NAME='" + pictureName + "' AND ALBUM_ID=" + std::to_string(id) + ";");
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	int id;
	const auto album = openAlbum(albumName);
	for (auto picture : album.getPictures())
	{
		if (picture.getName() == pictureName)
		{
			picture.tagUser(userId);
			tag_in_picture(userId, picture.getId());
		}
	}
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	int id;
	const auto album = openAlbum(albumName);
	for (auto picture : album.getPictures())
	{
		if (picture.getName() == pictureName)
		{
			picture.untagUser(userId);
			send_query("DELETE FROM TAGS WHERE PICTURE_ID=" + std::to_string(picture.getId()) + " AND USER_ID=" + std::to_string(userId) + ";");
		}
	}
}

void DatabaseAccess::printUsers()
{
	for (const auto& user : get_users())
	{
		std::cout << user << std::endl;
	}
}

void DatabaseAccess::createUser(User& user)
{
	auto id = 0;
	send_query("INSERT INTO USERS (NAME) VALUES ('" + user.getName() + "');");
	const auto query = "SELECT * FROM USERS WHERE NAME='" + user.getName() + "' ORDER BY ID DESC LIMIT 1;";
	sqlite3_exec(db_, query.c_str(), id_callback, &id, nullptr);
	user.setId(id);
}

void DatabaseAccess::deleteUser(const User& user)
{
	send_query("DELETE FROM USERS WHERE ID=" + std::to_string(user.getId()) + ";");
}

bool DatabaseAccess::doesUserExists(int userId)
{
	std::list<User> users;
	const auto query = "SELECT * FROM USERS WHERE ID=" + std::to_string(userId) + ";";
	sqlite3_exec(db_, query.c_str(), u_callback, &users, nullptr);
	if (users.empty())
	{
		return false;
	}
	users.clear();
	return true;
}

User DatabaseAccess::getUser(int userId)
{
	std::list<User> users;
	const auto query = "SELECT * FROM USERS WHERE ID=" + std::to_string(userId) + ";";
	sqlite3_exec(db_, query.c_str(), u_callback, &users, nullptr);
	if (users.empty())
	{
		throw ItemNotFoundException("User", userId);
	}
	for (auto user : users)
	{
		return user;
	}
	throw ItemNotFoundException("User", userId);
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	return getAlbumsOfUser(user).size();
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	int count;
	const auto query = "SELECT COUNT(DISTINCT ALBUM_ID) FROM PICTURES INNER JOIN TAGS ON TAGS.PICTURE_ID = PICTURES.ID WHERE TAGS.USER_ID=" + std::to_string(user.getId()) +";";
	sqlite3_exec(db_, query.c_str(), id_callback, &count, nullptr);
	return count;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	auto count = 0;
	const auto query = "SELECT COUNT(*) FROM TAGS WHERE USER_ID=" + std::to_string(user.getId()) + ";";
	sqlite3_exec(db_, query.c_str(), id_callback, &count, nullptr);
	return count;
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	auto albums = getAlbumsOfUser(user);
	auto total = 0;
	for (const auto& album : albums)
	{
		for (const auto& picture : album.getPictures())
		{
			total += picture.getTagsCount();
		}
	}
	if (total == 0)
	{
		return 0;
	}
	return static_cast<float>(total) / static_cast<float>(albums.size());
}

User DatabaseAccess::getTopTaggedUser()
{
	auto max = 0;
	auto users = get_users();
	User* max_user = nullptr;
	for (auto user : users)
	{
		const auto tags = countTagsOfUser(user);
		if (tags > max)
		{
			max = tags;
			max_user = &user;
		}
	}
	if (max == 0)
	{
		throw MyException("Failed to find most tagged user");
	}
	return *max_user;
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	std::list<Picture> pics;
	const std::string query = "SELECT * FROM PICTURES INNER JOIN TAGS ON PICTURES.ID = TAGS.PICTURE_ID WHERE PICTURES.ID = (SELECT PICTURE_ID FROM TAGS GROUP BY PICTURE_ID ORDER BY COUNT(USER_ID) DESC) LIMIT 1;";
	sqlite3_exec(db_, query.c_str(), p_callback, &pics, nullptr);
	for (auto pic : pics)
	{
		return pic;
	}
	throw MyException("There isn't any tagged picture.");
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	std::list<Picture> pics;
	const auto query = "SELECT * FROM PICTURES INNER JOIN TAGS ON PICTURES.ID = TAGS.PICTURE_ID WHERE TAGS.USER_ID = " + std::to_string(user.getId()) + ";";
	sqlite3_exec(db_, query.c_str(), p_callback, &pics, nullptr);
	return pics;
}

void DatabaseAccess::close()
{
	sqlite3_close(db_);
	db_ = nullptr;
}

void DatabaseAccess::clear()
{
	sqlite3_exec(db_, "DELETE FROM USERS", nullptr, nullptr, nullptr);
	sqlite3_exec(db_, "DELETE FROM ALBUMS", nullptr, nullptr, nullptr);
	sqlite3_exec(db_, "DELETE FROM PICTURES", nullptr, nullptr, nullptr);
	sqlite3_exec(db_, "DELETE FROM TAGS", nullptr, nullptr, nullptr);
}

int DatabaseAccess::a_callback(void* used, int argc, char** argv, char** az_col_name)
{
	Album* a;
	auto* albums = static_cast<std::list<Album>*>(used);
	if (argc == 1 && argv[0])
	{
		return 0;
	}
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

int DatabaseAccess::u_callback(void* used, int argc, char** argv, char** az_col_name)
{
	auto* users = static_cast<std::list<User>*>(used);
	if (argc == 0)
	{
		return 0;
	}
	users->push_back(User(atoi(argv[0]), argv[1]));
	return 0;
}

int DatabaseAccess::p_callback(void* used, int argc, char** argv, char** az_col_name)
{
	auto* pictures = static_cast<std::list<Picture>*>(used);
	if (argc == 0)
	{
		return 0;
	}
	pictures->push_back(Picture(atoi(argv[0]), argv[1], argv[2], argv[3]));
	return 0;
}

int DatabaseAccess::t_callback(void* used, int argc, char** argv, char** az_col_name)
{
	auto* tags = static_cast<std::list<int>*>(used);
	if (argc == 0)
	{
		return 0;
	}
	tags->push_back(atoi(argv[1]));
	return 0;
}

int DatabaseAccess::id_callback(void* used, int argc, char** argv, char** az_col_name)
{
	auto* id = static_cast<int*>(used);
	if (argc > 0)
	{
		*id = atoi(argv[0]);
	}
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

int DatabaseAccess::get_album_id(const std::string& album_name) const
{
	auto id = -1;
	const auto query = "SELECT * FROM ALBUMS WHERE NAME='" + album_name + "';"; 
	sqlite3_exec(db_, query.c_str(), id_callback, &id, nullptr);
	return id;
}

std::list<Picture> DatabaseAccess::get_pictures(int album_id) const
{
	std::list<Picture> pics;
	const auto query = "SELECT * FROM PICTURES WHERE ALBUM_ID=" + std::to_string(album_id) + ";";
	sqlite3_exec(db_, query.c_str(), p_callback, &pics, nullptr);
	for (auto& pic : pics)
	{
		for (auto user : get_tagged_users(pic.getId()))
		{
			pic.tagUser(user);
		}
	}
	return pics;
}

std::list<int> DatabaseAccess::get_tagged_users(int picture_id) const
{
	std::list<int> ids;
	const auto query = "SELECT * FROM TAGS WHERE PICTURE_ID=" + std::to_string(picture_id) + ";";
	sqlite3_exec(db_, query.c_str(), t_callback, &ids, nullptr);
	return ids;
}

void DatabaseAccess::tag_in_picture(int user_id, int picture_id) const
{
	send_query("INSERT INTO TAGS (PICTURE_ID, USER_ID) VALUES (" + std::to_string(picture_id) + ", " + std::to_string(user_id) + ");");
}

std::list<User> DatabaseAccess::get_users() const
{
	std::list<User> users;
	sqlite3_exec(db_, "SELECT * FROM USERS", u_callback, &users, nullptr);
	return users;
}
