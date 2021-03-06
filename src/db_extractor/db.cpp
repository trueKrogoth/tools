/*
 * AIM db_extractor
 * Copyright (C) 2015 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "db.h"

#include <common.h>

string getSqlType(uint32_t ft)
{
    switch (ft)
    {
    case T_STRING:
        return "TEXT";
    case T_INTEGER:
        return "INTEGER";
    case T_FLOAT:
        return "REAL";
    default:
        assert(false);
    }
    return "";
}

void table::load(buffer &b)
{
    READ(b, id);
    READ(b, name);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, unk4);
}

void field::load(buffer &b)
{
    READ(b, table_id);
    READ(b, id);
    READ(b, name);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, type);
}

void tab::load(buffer &b)
{
    READ(b, number_of_tables);
    READ(b, number_of_fields);

    auto n = number_of_tables;
    while (n--)
    {
        table t;
        t.load(b);
        tables[t.id] = t;
    }

    n = number_of_fields;
    while (n--)
    {
        field t;
        t.load(b);
        fields[t.id] = t;
    }
}

void value::load_index(buffer &b)
{
    READ(b, table_id);
    READ(b, name);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, offset);
    READ(b, data_size);
}

void value::load_data(buffer &b)
{
    data = buffer(b, data_size, offset);
}

void value::extract_fields(const tab &tab)
{
    while (!data.eof())
    {
        field_value fv;
        if (READ_NOTHROW(data, fv.field_id) == 0)
            break;
        READ(data, fv.size);
        auto i = tab.fields.find(fv.field_id);
        if (i == tab.fields.end())
            continue;
        switch (i->second.type)
        {
        case T_STRING:
            fv.s.resize(fv.size);
            READ_N(data, fv.s[0], fv.s.size());
            break;
        case T_INTEGER:
            READ(data, fv.i);
            if (fv.size > sizeof(fv.i))
                data.skip(fv.size - sizeof(fv.i));
            break;
        case T_FLOAT:
            READ(data, fv.f);
            if (fv.size > sizeof(fv.i))
                data.skip(fv.size - sizeof(fv.i));
            break;
        default:
            assert(false);
        }
        fields.push_back(fv);
    }
}

void db::load(buffer &b)
{
    READ(b, number_of_values);

    auto n = number_of_values;
    while (n--)
    {
        value t;
        t.load_index(b);
        values.push_back(t);
    }
}