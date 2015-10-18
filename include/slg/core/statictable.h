/***************************************************************************
 * Copyright 1998-2015 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#ifndef _SLG_STATICTABLE_H
#define	_SLG_STATICTABLE_H

#include <string>
#include <boost/unordered_map.hpp>

#include "luxrays/luxrays.h"
#include "luxrays/core/utils.h"

namespace slg {

template <class K, class T> class StaticTable {
public:
	StaticTable() { }
	~StaticTable() { }

	bool Get(const K &key, T &val) const {
		const boost::unordered_map<K, T> &table = GetTable();

		typename boost::unordered_map<K, T>::const_iterator it = table.find(key);
		if (it == table.end())
			return false;
		else {
			val = it->second;
			return true;
		}
	}

	class RegisterTableValue {
	public:
		RegisterTableValue(const K &key, const T &val) {
			boost::unordered_map<K, T> &table = GetTable();

			typename boost::unordered_map<K, T>::const_iterator it = table.find(key);
			if (it == table.end())
				table[key] = val;
			else
				throw std::runtime_error("Already registered key in StaticTable::RegisterTableValue::RegisterTableValue(): " + luxrays::ToString(key));
		}
		virtual ~RegisterTableValue() { }
	};

private:
	static boost::unordered_map<K, T> &GetTable() {
		// Using
		static boost::unordered_map<K, T> *table = new boost::unordered_map<K, T>();
		// instead of:
		//static boost::unordered_map<K, T> table;
		// to avoid possible allocation on thread heap.
		return *table;
	}
};

#define STATICTABLE_NAME(F) F ## _StaticTable

// Use STATICTABLE_DECLARE_DECLARATION inside the base class holding the table
#define STATICTABLE_DECLARE_DECLARATION(K, F) static StaticTable<K, F ## StaticTableType> STATICTABLE_NAME(F)
// Use STATICTABLE_DECLARATION inside the base class .cpp
#define STATICTABLE_DECLARATION(C, K, F) StaticTable<K, C::F ## StaticTableType> C::STATICTABLE_NAME(F)

// Use STATICTABLE_DECLARE_REGISTRATION() inside the class declaration to register
#define STATICTABLE_DECLARE_REGISTRATION(C, K, F) static StaticTable<K, F ## StaticTableType>::RegisterTableValue C ## F ## _StaticTableRegisterTableValue
// Use STATICTABLE_REGISTER() to register a class
// NOTE: you have to place all STATICTABLE_REGISTER() in the same .cpp file of the
// main base class (i.e. the one holding the StaticTable) because order of static
// field initialization is otherwise undefined.
#define STATICTABLE_REGISTER(R, C, N, K, F) StaticTable<K, R::F ## StaticTableType>::RegisterTableValue R::C ## F ## _StaticTableRegisterTableValue(N, C::F)

}

#endif	/* _SLG_STATICTABLE_H */
