/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2015 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "cylindrical.H"
#include "axesRotation.H"
#include "addToRunTimeSelectionTable.H"
#include "polyMesh.H"
#include "tensorIOField.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(cylindrical, 0);
    addToRunTimeSelectionTable
    (
        coordinateRotation,
        cylindrical,
        dictionary
    );
    addToRunTimeSelectionTable
    (
        coordinateRotation,
        cylindrical,
        objectRegistry
    );
}


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

void Foam::cylindrical::init
(
    const objectRegistry& obr,
    const List<label>& cells
)
{
    const polyMesh& mesh = refCast<const polyMesh>(obr);
    const vectorField& cc = mesh.cellCentres();

    if (cells.size())
    {
        Rptr_.reset(new tensorField(cells.size()));

        tensorField& R = Rptr_();
        forAll(cells, i)
        {
            label cellI = cells[i];
            vector dir = cc[cellI] - origin_;
            dir /= mag(dir) + VSMALL;

            R[i] = axesRotation(e3_, dir).R();
        }
    }
    else
    {
        Rptr_.reset(new tensorField(mesh.nCells()));

        tensorField& R = Rptr_();
        forAll(cc, cellI)
        {
            vector dir = cc[cellI] - origin_;
            dir /= mag(dir) + VSMALL;

            R[cellI] = axesRotation(e3_, dir).R();
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::cylindrical::cylindrical
(
    const dictionary& dict,
    const objectRegistry& obr
)
:
    Rptr_(),
    origin_(point::zero),
    e3_(vector::zero)
{
    // If origin is specified in the coordinateSystem
    if (dict.parent().found("origin"))
    {
        dict.parent().lookup("origin") >> origin_;
    }

    // rotation axis
    dict.lookup("e3") >> e3_;

    init(obr);
}


Foam::cylindrical::cylindrical
(
    const objectRegistry& obr,
    const vector& axis,
    const point& origin
)
:
    Rptr_(),
    origin_(origin),
    e3_(axis)
{
    init(obr);
}


Foam::cylindrical::cylindrical
(
    const objectRegistry& obr,
    const vector& axis,
    const point& origin,
    const List<label>& cells
)
:
    Rptr_(),
    origin_(origin),
    e3_(axis)
{
    init(obr, cells);
}


Foam::cylindrical::cylindrical(const dictionary& dict)
:
    Rptr_(),
    origin_(),
    e3_()
{
    FatalErrorIn("cylindrical(const dictionary&)")
        << " cylindrical can not be constructed from dictionary "
        << " use the construtctor : "
           "("
           "    const dictionary&, const objectRegistry&"
           ")"
        << exit(FatalIOError);
}


Foam::cylindrical::cylindrical(const tensorField& R)
:
    Rptr_(),
    origin_(vector::zero),
    e3_(vector::zero)
{
    Rptr_() = R;
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::cylindrical::clear()
{
    if (!Rptr_.empty())
    {
        Rptr_.clear();
    }
}


void Foam::cylindrical::updateCells
(
    const polyMesh& mesh,
    const labelList& cells
)
{
    const vectorField& cc = mesh.cellCentres();
    tensorField& R = Rptr_();

    forAll(cells, i)
    {
        label cellI = cells[i];
        vector dir = cc[cellI] - origin_;
        dir /= mag(dir) + VSMALL;

        R[cellI] = axesRotation(e3_, dir).R();
    }
}


Foam::tmp<Foam::vectorField> Foam::cylindrical::transform
(
    const vectorField& vf
) const
{
    if (Rptr_->size() != vf.size())
    {
        FatalErrorIn
        (
            "tmp<vectorField> cylindrical::transform(const vectorField&)"
        )
            << "vectorField st has different size to tensorField "
            << abort(FatalError);
    }

    return (Rptr_() & vf);
}


Foam::vector Foam::cylindrical::transform(const vector& v) const
{
    notImplemented
    (
        "vector cylindrical::transform(const vector&) const"
    );
    return vector::zero;
}


Foam::vector Foam::cylindrical::transform
(
    const vector& v,
    const label cmptI
) const
{
    return (Rptr_()[cmptI] & v);
}


Foam::tmp<Foam::vectorField> Foam::cylindrical::invTransform
(
    const vectorField& vf
) const
{
    return (Rptr_().T() & vf);
}


Foam::vector Foam::cylindrical::invTransform(const vector& v) const
{
    notImplemented
    (
        "vector cylindrical::invTransform(const vector&) const"
    );
    return vector::zero;
}


Foam::vector Foam::cylindrical::invTransform
(
    const vector& v,
    const label cmptI
) const
{
    return (Rptr_()[cmptI].T() & v);
}


Foam::tmp<Foam::tensorField> Foam::cylindrical::transformTensor
(
    const tensorField& tf
) const
{
    if (Rptr_->size() != tf.size())
    {
        FatalErrorIn
        (
            "tmp<tensorField> cylindrical::transformTensor"
            "("
                "const tensorField&"
            ")"
        )
            << "tensorField st has different size to tensorField Tr"
            << abort(FatalError);
    }
    return (Rptr_() & tf & Rptr_().T());
}


Foam::tensor Foam::cylindrical::transformTensor
(
    const tensor& t
) const
{
    notImplemented
    (
        "tensor cylindrical::transformTensor(const tensor&) const"
    );

    return tensor::zero;
}


Foam::tmp<Foam::tensorField> Foam::cylindrical::transformTensor
(
    const tensorField& tf,
    const labelList& cellMap
) const
{
    if (cellMap.size() != tf.size())
    {
        FatalErrorIn
        (
            "tmp<tensorField> cylindrical::transformTensor"
            "("
                "const tensorField&, "
                "const labelList&"
            ")"
        )
            << "tensorField tf has different size to tensorField Tr"
            << abort(FatalError);
    }

    const tensorField& R = Rptr_();
    const tensorField Rtr(R.T());
    tmp<tensorField> tt(new tensorField(cellMap.size()));
    tensorField& t = tt();
    forAll(cellMap, i)
    {
        const label cellI = cellMap[i];
        t[i] = R[cellI] & tf[i] & Rtr[cellI];
    }

    return tt;
}


Foam::tmp<Foam::symmTensorField> Foam::cylindrical::transformVector
(
    const vectorField& vf
) const
{
    if (Rptr_->size() != vf.size())
    {
        FatalErrorIn("cylindrical::transformVector(const vectorField&)")
            << "tensorField vf has different size to tensorField Tr"
            << abort(FatalError);
    }

    tmp<symmTensorField> tfld(new symmTensorField(Rptr_->size()));
    symmTensorField& fld = tfld();

    const tensorField& R = Rptr_();
    forAll(fld, i)
    {
        fld[i] = transformPrincipal(R[i], vf[i]);
    }
    return tfld;
}


Foam::symmTensor Foam::cylindrical::transformVector
(
    const vector& v
) const
{
    notImplemented
    (
        "tensor cylindrical::transformVector(const vector&) const"
    );
    return symmTensor::zero;
}


void Foam::cylindrical::write(Ostream& os) const
{
     os.writeKeyword("e3") << e3() << token::END_STATEMENT << nl;
}


// ************************************************************************* //
