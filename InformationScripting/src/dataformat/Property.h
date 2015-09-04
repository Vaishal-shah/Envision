/***********************************************************************************************************************
**
** Copyright (c) 2011, 2015 ETH Zurich
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
** following conditions are met:
**
**    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
**      disclaimer.
**    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
**      following disclaimer in the documentation and/or other materials provided with the distribution.
**    * Neither the name of the ETH Zurich nor the names of its contributors may be used to endorse or promote products
**      derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
***********************************************************************************************************************/

#pragma once

#include "../informationscripting_api.h"

namespace Model {
	class Node;
}

namespace InformationScripting {

// Inspired by: http://channel9.msdn.com/Events/GoingNative/2013/Inheritance-Is-The-Base-Class-of-Evil
class INFORMATIONSCRIPTING_API Property {
	public:
		// TODO: in the future we might need a copy constructor, and clone methods in the PropertyDataConcept.
		// This depends on how we use Properties, for now data is always shared when copying Properties.
		Property() = default;
		template <class DataType> Property(DataType propertyData);

		friend boost::python::object pythonObject(const Property& p);

		template <class ConvertTo> operator ConvertTo() const;
		operator Model::Node*() const;

		bool operator==(const Property& other) const;

		uint hash(uint seed = 0) const;

	private:
		struct PropertyDataConcept {
				virtual ~PropertyDataConcept() = default;
				virtual boost::python::object pythonObject() const = 0;
				virtual Model::Node* node() const { return nullptr; }
				virtual bool equals(const std::shared_ptr<PropertyDataConcept>& other) const = 0;
				virtual uint hash(uint seed = 0) const = 0;
		};

		template <class DataType, class = void>
		struct PropertyData : PropertyDataConcept {
				PropertyData(DataType data) : data_{std::move(data)} {}

				virtual boost::python::object pythonObject() const override {
					return boost::python::object(data_);
				}

				virtual bool equals(const std::shared_ptr<PropertyDataConcept>& other) const override {
					if (auto specificOther = std::dynamic_pointer_cast<PropertyData<DataType>>(other))
						return data_ == specificOther->data_;
					return false;
				}

				virtual uint hash(uint seed) const override {
					return qHash(data_, seed);
				}

				DataType data_;
		};
		// Template overload for general pointer types:
		template <class DataType>
		struct PropertyData<DataType, typename std::enable_if<std::is_pointer<DataType>::value
								&& !std::is_base_of<Model::Node, std::remove_pointer_t<DataType>>::value>::type>
				: PropertyDataConcept {
				PropertyData(DataType data) : data_{data} {}

				virtual boost::python::object pythonObject() const override {
					return boost::python::object(boost::python::ptr(data_));
				}

				virtual bool equals(const std::shared_ptr<PropertyDataConcept>& other) const override {
					if (auto specificOther = std::dynamic_pointer_cast<PropertyData<DataType>>(other))
						return data_ == specificOther->data_;
					return false;
				}

				virtual uint hash(uint seed) const override {
					return qHash(data_, seed);
				}

				DataType data_;
		};
		// Template overload for pointer types which inherit from Model::Node
		template <class DataType>
		struct PropertyData<DataType, typename std::enable_if<std::is_pointer<DataType>::value
								&& std::is_base_of<Model::Node, std::remove_pointer_t<DataType>>::value>::type>
				: PropertyDataConcept {
				PropertyData(DataType data) : data_{data} {}

				virtual boost::python::object pythonObject() const override {
					return boost::python::object(boost::python::ptr(data_));
				}
				virtual Model::Node* node() const override { return data_; }

				virtual bool equals(const std::shared_ptr<PropertyDataConcept>& other) const override {
					// Just compare node pointers since they are unique.
					if (node()) return node() == other->node();
					// If we have a nullptr, check if other has the same type and value.
					if (auto specificOther = std::dynamic_pointer_cast<PropertyData<DataType>>(other))
						return data_ == specificOther->data_;
					return false;
				}

				virtual uint hash(uint seed) const override {
					return qHash(data_, seed);
				}

				DataType data_;
		};

		std::shared_ptr<PropertyDataConcept> data_;
};

struct INFORMATIONSCRIPTING_API NamedProperty : QPair<QString, Property>
{
	NamedProperty() = default;
	NamedProperty(const QString& key, Property value) : QPair<QString, Property>(key, value) {}
};

template <class DataType> Property::Property(DataType propertyData)
	: data_{std::make_shared<PropertyData<DataType>>(std::move(propertyData))} {}

template <class ConvertTo>
inline Property::operator ConvertTo() const
{
	if (auto propertyData = std::dynamic_pointer_cast<PropertyData<ConvertTo>>(data_))
		return propertyData->data_;
	throw new std::bad_cast;
}

inline InformationScripting::Property::operator Model::Node*() const { return data_->node(); }

// qHash functions have to accessible outside:
uint qHash(const InformationScripting::Property& p, uint seed = 0);

} /* namespace InformationScripting */
