#ifndef OWLAPI_MODEL_OWL_NAMED_OBJECT_HPP
#define OWLAPI_MODEL_OWL_NAMED_OBJECT_HPP

#include <owlapi/model/OWLObject.hpp>
#include <owlapi/model/HasIRI.hpp>

namespace owlapi {
namespace model {

class OWLNamedObject : public OWLObject, HasIRI
{
    IRI mIRI;

public:
    typedef shared_ptr<OWLNamedObject> Ptr;

    OWLNamedObject( const IRI& iri = IRI())
        : OWLObject()
        , HasIRI()
        , mIRI(iri)
    {}

    virtual ~OWLNamedObject() {}
   
    IRI getIRI() const { return mIRI; }

    void setIRI(const IRI& iri) { mIRI = iri; }
};

} // end namespace model
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_NAMED_OBJECT_HPP
