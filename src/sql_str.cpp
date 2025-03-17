// c++可以有原始字符串，所以用C++

extern "C" const char* create_table_sql = R"(
-- 物品类别表
CREATE TABLE categorys (
	id	INTEGER NOT NULL UNIQUE,
	sort	INTEGER,
	name	VARCHAR(32) NOT NULL,
	PRIMARY KEY(id AUTOINCREMENT)
);
INSERT INTO categorys (sort,name) VALUES (1,'办公用品');
INSERT INTO categorys (sort,name) VALUES (2,'卫生用品');
INSERT INTO categorys (sort,name) VALUES (3,'水电五金');
INSERT INTO categorys (sort,name) VALUES (4,'体育用品');
INSERT INTO categorys (sort,name) VALUES (5,'电子信息');
INSERT INTO categorys (sort,name) VALUES (6,'其它物品');

-- 库位表
CREATE TABLE places (
	id	INTEGER NOT NULL UNIQUE,
	sort	INTEGER,
	name	VARCHAR(32) NOT NULL,
	note	VARCHAR(256),
	PRIMARY KEY(id AUTOINCREMENT)
);
INSERT INTO places (sort,name) VALUES (1,'A0-0');
INSERT INTO places (sort,name) VALUES (2,'A1-0');
INSERT INTO places (sort,name) VALUES (3,'A1-1');
INSERT INTO places (sort,name) VALUES (3,'A1-2');
INSERT INTO places (sort,name) VALUES (3,'A1-3');

-- 物品信息表
CREATE TABLE items (
	id			INTEGER NOT NULL UNIQUE,
	name		VARCHAR(64) NOT NULL UNIQUE,
	cat_id		INTEGER NOT NULL DEFAULT 0,
	frequency	INTEGER DEFAULT 0,
	unit		VARCHAR(32) NOT NULL,
	place_id	INTEGER NOT NULL DEFAULT 0,
	amount		INTEGER ,
	alarm		INTEGER  DEFAULT -1,
	image 		BLOB,
	PRIMARY KEY(id AUTOINCREMENT)
);

INSERT INTO items (name,cat_id,frequency,unit,place_id,amount,alarm) VALUES ('A4打印纸',1,0,'包',0,0,8);
INSERT INTO items (name,cat_id,frequency,unit,place_id,amount,alarm) VALUES ('粉色A4打印纸',1,0,'包',0,0,2);

-- 库存记录表
CREATE TABLE stock_recodes (
	id			INTEGER NOT NULL UNIQUE,
	date		DATE NOT NULL,
	word		CHAR(8) NOT NULL,
	code		INTEGER NOT NULL,
	who			VARCHAR(256) NOT NULL,
	item_id		INTEGER NOT NULL,
	stock_in	INTEGER DEFAULT 0,
	stock_out	INTEGER DEFAULT 0,
	amount		INTEGER DEFAULT 0,
	note		VARCHAR(256),
	PRIMARY KEY(id AUTOINCREMENT)
);

-- 电子符件表
CREATE TABLE attachments (
	id			INTEGER NOT NULL UNIQUE,
	voucher_id	TEXT NOT NULL,
	filename	TEXT,
	format		TEXT,
	data		BLOB NOT NULL,
	PRIMARY KEY(id AUTOINCREMENT)
);
-- 供应商表
CREATE TABLE vendors (
	id			INTEGER NOT NULL UNIQUE,
	name		VARCHAR(32) NOT NULL,
	phone		VARCHAR(32),
	useing		INTEGER DEFAULT 1,
	PRIMARY 	KEY(id AUTOINCREMENT)
);
INSERT INTO vendors (name,phone) VALUES ('高俊强',"000");
INSERT INTO vendors (name,phone) VALUES ('海燕',"000");
INSERT INTO vendors (name,phone) VALUES ('王文亮',"000");

-- 部门表
CREATE TABLE sectors (
	id			INTEGER NOT NULL UNIQUE,
	name		VARCHAR(32) NOT NULL,
	phone		VARCHAR(32),
	useing		INTEGER DEFAULT 1,
	PRIMARY 	KEY(id AUTOINCREMENT)
);
INSERT INTO sectors (name,phone) VALUES ('不确定的部门',"000");
INSERT INTO sectors (name,phone) VALUES ('22级 一部',"000");
INSERT INTO sectors (name,phone) VALUES ('22级 二部',"000");
INSERT INTO sectors (name,phone) VALUES ('23级 来村部',"000");

-- 设置参数表
CREATE TABLE seting (
	id		INTEGER NOT NULL UNIQUE,
	name	VARCHAR(32) NOT NULL UNIQUE,
	valume	TEXT  NOT NULL,
	PRIMARY KEY(id AUTOINCREMENT)
);
INSERT INTO seting (name,valume) VALUES ('单位名称',"为学网");
INSERT INTO seting (name,valume) VALUES ('库房名称',"主库");
INSERT INTO seting (name,valume) VALUES ('init_date',date());

)";