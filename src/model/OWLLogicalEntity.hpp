#ifndef OWLAPI_MODEL_OWL_LOGICAL_ENTITY_HPP
#define OWLAPI_MODEL_OWL_LOGICAL_ENTITY_HPP

#include "OWLEntity.hpp"

namespace owlapi {
namespace model {

class OWLLogicalEntity : public OWLEntity
{
public:
    OWLLogicalEntity(const IRI& iri, OWLEntity::EntityType type)
        : OWLEntity(iri, type)
    {}

    virtual ~OWLLogicalEntity() {}
};

} // end namespace model
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_LOGICAL_ENTITY_HPP
