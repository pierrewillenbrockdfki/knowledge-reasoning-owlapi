#include "OWLOntologyAsk.hpp"
#include <owlapi/KnowledgeBase.hpp>

#include <owlapi/Vocabulary.hpp>

namespace owlapi {
namespace model {

OWLOntologyAsk::OWLOntologyAsk(OWLOntology::Ptr ontology)
    : mpOntology(ontology)
{}

OWLClass::Ptr OWLOntologyAsk::getOWLClass(const IRI& iri) const
{
    std::map<IRI, OWLClass::Ptr>::const_iterator it = mpOntology->mClasses.find(iri);
    if(it != mpOntology->mClasses.end())
    {
        return it->second;
    }

    throw std::runtime_error("OWLOntologyAsk::getOWLClass: '" + iri.toString() + "' is not a known OWLClass");
}

OWLIndividual::Ptr OWLOntologyAsk::getOWLIndividual(const IRI& iri) const
{
    try {
        return getOWLAnonymousIndividual(iri);
    } catch(const std::runtime_error& e)
    {}

    try {
        return getOWLNamedIndividual(iri);
    } catch(const std::runtime_error& e)
    {}

    throw std::runtime_error("OWLOntologyAsk::getOWLIndividual: '" + iri.toString() + "' is not a known OWLIndividual");
}

OWLAnonymousIndividual::Ptr OWLOntologyAsk::getOWLAnonymousIndividual(const IRI& iri) const
{
    std::map<IRI, OWLAnonymousIndividual::Ptr>::const_iterator it = mpOntology->mAnonymousIndividuals.find(iri);
    if(it != mpOntology->mAnonymousIndividuals.end())
    {
        return it->second;
    }
    throw std::runtime_error("OWLOntologyAsk::getAnonymousIndividual: '" + iri.toString() + "' is not a known OWLAnonymousIndividual");
}

OWLNamedIndividual::Ptr OWLOntologyAsk::getOWLNamedIndividual(const IRI& iri) const
{
    std::map<IRI, OWLNamedIndividual::Ptr>::const_iterator it = mpOntology->mNamedIndividuals.find(iri);
    if(it != mpOntology->mNamedIndividuals.end())
    {
        return it->second;
    } 

    throw std::runtime_error("OWLOntologyAsk::getNamedIndividual: '" + iri.toString() + "' is not a known OWLNamedIndividual");
}

OWLObjectProperty::Ptr OWLOntologyAsk::getOWLObjectProperty(const IRI& iri) const
{
    std::map<IRI, OWLObjectProperty::Ptr>::const_iterator it = mpOntology->mObjectProperties.find(iri);
    if(it != mpOntology->mObjectProperties.end())
    {
        return it->second;
    }
    throw std::runtime_error("OWLOntologyAsk::getOWLObjectProperty: '" + iri.toString() + "' is not a known OWLObjectProperty");
}

OWLDataProperty::Ptr OWLOntologyAsk::getOWLDataProperty(const IRI& iri) const
{
    std::map<IRI, OWLDataProperty::Ptr>::const_iterator it = mpOntology->mDataProperties.find(iri);
    if(it != mpOntology->mDataProperties.end())
    {
        return it->second;
    }

    throw std::runtime_error("OWLOntologyAsk::getOWLDataProperty: '" + iri.toString() + "' is not a known OWLDataProperty");
}

std::vector<OWLCardinalityRestriction::Ptr> OWLOntologyAsk::getCardinalityRestrictions(owlapi::model::OWLClassExpression::Ptr ce) const
{
    // In order to find a restriction for a given class
    //    1. check class assertions for individuals
    // -> 2. check subclass axioms for classes
    //      - find superclass definitions, collect all restrictions
    //        - (including the ones for the superclasses -- identify restrictions)
    std::vector<OWLSubClassOfAxiom::Ptr> subclassAxioms = mpOntology->mSubClassAxiomBySubPosition[ce];

    std::vector<OWLCardinalityRestriction::Ptr> restrictions;
    std::vector<OWLSubClassOfAxiom::Ptr>::const_iterator sit = subclassAxioms.begin();
    for(; sit != subclassAxioms.end(); ++sit)
    {
        OWLSubClassOfAxiom::Ptr subclassAxiomPtr = *sit;
        OWLClassExpression::Ptr superClass = subclassAxiomPtr->getSuperClass();

        // The class for which the cardinality expressions are queried for
        OWLClassExpression::Ptr subClass = subclassAxiomPtr->getSubClass();

        switch(superClass->getClassExpressionType())
        {
            case OWLClassExpression::OBJECT_MIN_CARDINALITY:
            case OWLClassExpression::OBJECT_MAX_CARDINALITY:
            case OWLClassExpression::OBJECT_EXACT_CARDINALITY:
            case OWLClassExpression::DATA_EXACT_CARDINALITY:
            case OWLClassExpression::DATA_MIN_CARDINALITY:
            case OWLClassExpression::DATA_MAX_CARDINALITY:
            {
                OWLCardinalityRestriction::Ptr restriction = dynamic_pointer_cast<OWLCardinalityRestriction>(superClass);

                std::vector<OWLCardinalityRestriction::Ptr> inheritedRestrictions = getCardinalityRestrictions(restriction->getQualification());

                if(inheritedRestrictions.empty())
                {
                    restrictions.push_back(dynamic_pointer_cast<OWLCardinalityRestriction>(superClass)->clone());
                } else {
                    // We need to increase scale each cardinality based on the
                    // requirements of the restrictions that got it in here in
                    // the first place
                    std::vector<OWLCardinalityRestriction::Ptr> scaledRestrictions = OWLCardinalityRestriction::scale(inheritedRestrictions,
                            restriction->getCardinality());

                    restrictions = OWLCardinalityRestriction::intersection(restrictions, scaledRestrictions);
                }
                break;
            }
            case OWLClassExpression::OWL_CLASS:
            {
                std::vector<OWLCardinalityRestriction::Ptr> inheritedRestrictions = getCardinalityRestrictions(superClass);
                restrictions = OWLCardinalityRestriction::intersection(restrictions, inheritedRestrictions);
            }
            default:
                break;
        }
    }
    return OWLCardinalityRestriction::compact(restrictions);
}

std::vector<OWLCardinalityRestriction::Ptr> OWLOntologyAsk::getCardinalityRestrictions(const IRI& iri) const
{
    // In order to find a restriction for a given class
    //    1. check class assertions for individuals
    // -> 2. check subclass axioms for classes
    //      - find superclass definitions, collect all restrictions
    //        - (including the ones for the superclasses -- identify restrictions)
    OWLClass::Ptr klass = getOWLClass(iri);
    return getCardinalityRestrictions(klass);
}

std::vector<OWLCardinalityRestriction::Ptr> OWLOntologyAsk::getCardinalityRestrictions(const std::vector<IRI>& klasses) const
{
    std::vector<IRI>::const_iterator cit = klasses.begin();
    std::vector<OWLCardinalityRestriction::Ptr> restrictions;

    for(; cit != klasses.end(); ++cit)
    {
        IRI iri = *cit;
        std::vector<OWLCardinalityRestriction::Ptr> klassRestrictions = getCardinalityRestrictions(iri);
        restrictions = OWLCardinalityRestriction::join(restrictions, klassRestrictions);
    }
    return restrictions;
}

bool OWLOntologyAsk::isSubClassOf(const IRI& iri, const IRI& superclass) const
{
    return mpOntology->kb()->isSubClassOf(iri, superclass);
}

bool OWLOntologyAsk::isSubClassOf(const OWLClassExpression::Ptr& subclass, const OWLClassExpression::Ptr& superclass) const
{
    IRI subclassIRI;
    IRI superclassIRI;

    if(subclass->isClassExpressionLiteral() && superclass->isClassExpressionLiteral())
    {
        subclassIRI = dynamic_pointer_cast<OWLClass>(subclass)->getIRI();
        superclassIRI = dynamic_pointer_cast<OWLClass>(superclass)->getIRI();

        return isSubClassOf(subclassIRI, superclassIRI);
    }

    throw std::runtime_error("owlapi::model::OWLOntologyAsk::isSubClassOf:"
            " cannot (yet) handle anyonmous class definitions");
}

bool OWLOntologyAsk::isOWLClass(const IRI& iri) const
{
    std::map<IRI, OWLClass::Ptr>::const_iterator it = mpOntology->mClasses.find(iri);
    return it != mpOntology->mClasses.end();
}

IRIList OWLOntologyAsk::allInstancesOf(const IRI& classType, bool direct) const
{
    return mpOntology->kb()->allInstancesOf(classType, direct);
}

IRIList OWLOntologyAsk::allSubClassesOf(const IRI& classType, bool direct) const
{
    return mpOntology->kb()->allSubClassesOf(classType, direct);
}

IRIList OWLOntologyAsk::allInstances() const
{
    return mpOntology->kb()->allInstances();
}

IRIList OWLOntologyAsk::allClasses(bool excludeBottomClass) const
{
    return mpOntology->kb()->allClasses(excludeBottomClass);
}

IRIList OWLOntologyAsk::allObjectProperties() const
{
    IRIList list;
    std::map<IRI, OWLObjectProperty::Ptr>::const_iterator cit = mpOntology->mObjectProperties.begin();
    for(; cit != mpOntology->mObjectProperties.end(); ++cit)
    {
        list.push_back(cit->first);
    }
    return list;
}

IRIList OWLOntologyAsk::allDataProperties() const
{
    IRIList list;
    std::map<IRI, OWLDataProperty::Ptr>::const_iterator cit = mpOntology->mDataProperties.begin();
    for(; cit != mpOntology->mDataProperties.end(); ++cit)
    {
        list.push_back(cit->first);
    }
    return list;
}

bool OWLOntologyAsk::isInstanceOf(const IRI& instance, const IRI& klass) const
{
    return mpOntology->kb()->isInstanceOf(instance, klass);
}

bool OWLOntologyAsk::isRelatedTo(const IRI& instance, const IRI& relationProperty, const IRI& otherInstance) const
{
    return mpOntology->kb()->isRelatedTo(instance, relationProperty, otherInstance);
}

IRI OWLOntologyAsk::typeOf(const IRI& instance) const
{
    return mpOntology->kb()->typeOf(instance);
}

IRIList OWLOntologyAsk::allRelatedInstances(const IRI& instance, const IRI& relationProperty, const IRI& klass) const
{
    return mpOntology->kb()->allRelatedInstances(instance, relationProperty, klass);
}

IRIList OWLOntologyAsk::allInverseRelatedInstances(const IRI& instance, const IRI& relationProperty, const IRI& klass) const
{
    return mpOntology->kb()->allInverseRelatedInstances(instance, relationProperty, klass);
}


OWLLiteral::Ptr OWLOntologyAsk::getDataValue(const IRI& instance, const IRI& dataProperty) const
{
    reasoner::factpp::DataValue dataValue = mpOntology->kb()->getDataValue(instance, dataProperty);

    std::string valueRepresentation = dataValue.getValue();
    if(!dataValue.getType().empty())
    {
        valueRepresentation += "^^" + dataValue.getType();
    }

    return OWLLiteral::create(valueRepresentation);
}

IRIList OWLOntologyAsk::getDataPropertyDomain(const IRI& dataProperty, bool direct) const
{
    return mpOntology->kb()->getDataPropertyDomain(dataProperty, direct);
}

OWLDataRange::PtrList OWLOntologyAsk::getDataRange(const IRI& dataProperty) const
{
    std::map<IRI, OWLDataProperty::Ptr>::const_iterator cit = mpOntology->mDataProperties.find(dataProperty);
    if(cit != mpOntology->mDataProperties.end())
    {
        const OWLDataProperty::Ptr& property = cit->second;
        return property->getDataRanges();
    }
    throw std::invalid_argument("owlapi::model::OWLOntologyAsk::getDataRange: "
            " no data property '" + dataProperty.toString() + "' found");
}

IRIList OWLOntologyAsk::getObjectPropertyDomain(const IRI& objectProperty, bool direct) const
{
    return mpOntology->kb()->getObjectPropertyDomain(objectProperty, direct);
}

IRIList OWLOntologyAsk::ancestors(const IRI& instance) const
{
    return mpOntology->kb()->typesOf(instance);
}

bool OWLOntologyAsk::isObjectProperty(const IRI& property) const
{
    try {
        mpOntology->kb()->getObjectProperty(property);
        return true;
    } catch(const std::exception& e)
    {
        return false;
    }
}

bool OWLOntologyAsk::isDataProperty(const IRI& property) const
{
    try {
        mpOntology->kb()->getDataProperty(property);
        return true;
    } catch(const std::exception& e)
    {
        return false;
    }
}

IRIList OWLOntologyAsk::getObjectPropertiesForDomain(const IRI& domain) const
{
    IRIList validProperties;
    IRIList objectProperties = mpOntology->kb()->allObjectProperties();
    IRIList::const_iterator cit = objectProperties.begin();
    for(; cit != objectProperties.end(); ++cit)
    {
        const IRI& property = *cit;
        IRIList domains = domainOf(property);
        if( isSubClassOfIntersection(domain, domains) )
        {
            validProperties.push_back(property);
        }
    }
    return validProperties;
}

IRIList OWLOntologyAsk::getDataPropertiesForDomain(const IRI& domain) const
{
    IRIList validProperties;
    IRIList dataProperties = mpOntology->kb()->allDataProperties();
    IRIList::const_iterator cit = dataProperties.begin();
    for(; cit != dataProperties.end(); ++cit)
    {
        const IRI& property = *cit;
        IRIList domains = domainOf(property);
        if(isSubClassOfIntersection(domain, domains) )
        {
            validProperties.push_back(property);
        }
    }
    return validProperties;
}

bool OWLOntologyAsk::isSubClassOfIntersection(const IRI& klass, const IRIList& intersection) const
{
    // TODO: Proper check on intersection
    IRIList::const_iterator cit = intersection.begin();
    for(; cit != intersection.end(); ++cit)
    {
        if(isSubClassOf(klass, *cit))
        {
            return true;
        }
    }

    return false;
}

IRIList OWLOntologyAsk::domainOf(const IRI& iri, bool direct) const
{
    IRIList domains;
    if(isObjectProperty(iri))
    {
        return getObjectPropertyDomain(iri, direct);
    } else if(isDataProperty(iri))
    {
        return getDataPropertyDomain(iri, direct);
    }

    throw std::invalid_argument("owlapi::model::OWLOntologyAsk::domainOf:"
            " property '" + iri.toString() + "' is not known");
}

IRIList OWLOntologyAsk::imports() const
{
    return mpOntology->mDirectImportsDocuments;
}

} // end namespace model
} // end namespace owlapi
