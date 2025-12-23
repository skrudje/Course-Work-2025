#ifndef DB_SCHEMA_H
#define DB_SCHEMA_H

#include <QString>

// 1. скрипт создания таблицы
const QString SQL_CREATE_TABLES = R"(
    CREATE TABLE IF NOT EXISTS hosts (
        ip TEXT PRIMARY KEY,
        mac TEXT,
        type TEXT,
        scan_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
)";

// 2. хранимая процедура для добавления/обновления хоста
const QString SQL_FUNC_ADD_HOST = R"(
    CREATE OR REPLACE FUNCTION sp_add_host(_ip TEXT, _mac TEXT, _type TEXT)
    RETURNS VOID
    LANGUAGE plpgsql
    AS $$
    BEGIN
        INSERT INTO hosts (ip, mac, type, scan_date)
        VALUES (_ip, _mac, _type, CURRENT_TIMESTAMP)
        ON CONFLICT (ip) DO UPDATE SET
            mac = EXCLUDED.mac,
            type = EXCLUDED.type,
            scan_date = CURRENT_TIMESTAMP;
    END;
    $$;
)";

// 3. Хранимая функция для получения всех хостов
const QString SQL_FUNC_GET_HOSTS = R"(
    CREATE OR REPLACE FUNCTION sp_get_all_hosts()
    RETURNS TABLE(o_ip TEXT, o_mac TEXT, o_type TEXT) 
    LANGUAGE sql
    AS $$
        SELECT ip, mac, type FROM hosts ORDER BY scan_date DESC;
    $$;
)";

// 4. Процедура очистки
const QString SQL_FUNC_CLEAR = R"(
    CREATE OR REPLACE FUNCTION sp_clear_hosts()
    RETURNS VOID
    LANGUAGE sql
    AS $$
        TRUNCATE TABLE hosts;
    $$;
)";

// 5. создание таблицы справочника производителей
const QString SQL_CREATE_VENDORS = R"(
    CREATE TABLE IF NOT EXISTS vendors (
        prefix VARCHAR(20) PRIMARY KEY,
        vendor_name TEXT
    );
)";

// 6. заполнение справочника
const QString SQL_SEED_VENDORS = R"(
    INSERT INTO vendors (prefix, vendor_name) VALUES
    ('00:17:F2', 'Apple'),
    ('00:25:00', 'Apple'),
    ('00:12:47', 'Samsung'),
    ('34:14:5F', 'Samsung'),
    ('00:1D:7E', 'TP-Link'),
    ('C0:4A:00', 'TP-Link'),
    ('04:D9:F5', 'Asus'),
    ('E8:94:F6', 'Keenetic'),
    ('00:1B:21', 'Intel'),
    ('00:D8:61', 'MSI'),
    ('08:00:27', 'VirtualBox'),
    ('00:0C:29', 'VMware'),
    ('00:50:56', 'VMware')
    ON CONFLICT (prefix) DO NOTHING;
)";

// 7. функция поиска производителя (берет мак, отрезает первые 8 символов и ищет в таблице)
const QString SQL_FUNC_GET_VENDOR = R"(
    CREATE OR REPLACE FUNCTION sp_get_vendor(_mac TEXT)
    RETURNS TEXT
    LANGUAGE plpgsql
    AS $$
    DECLARE
        _prefix TEXT;
        _name TEXT;
    BEGIN
        -- Берём первые 8 символов (XX:XX:XX) и приводим к верхнему регистру
        _prefix := UPPER(SUBSTRING(_mac, 1, 8));
        
        SELECT vendor_name INTO _name FROM vendors WHERE prefix = _prefix;
        
        IF _name IS NULL THEN
            RETURN 'Unknown';
        ELSE
            RETURN _name;
        END IF;
    END;
    $$;
)";

#endif // DB_SCHEMA_H