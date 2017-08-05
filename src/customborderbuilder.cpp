// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

/*
Border-Builder 09.09.2008
by Stefan Kriwanek
contact@stefankriwanek.de

erstellt mit libsiedler2 rev 3935

Die Methode loadEdges() der Klasse lädt die zum Erstellen eines Rahmens nötigen Bitmaps in den Speicher.
Übergeben wird
archiveInfo:    Zeiger auf ein geladenes ArchivInfo der RESOURCE.DAT/IDX

Die Methode buildBorder() erstellt einen Rahmen zu gegebener Größe.
Übergeben werden:
width, height:  die gewünschte Größe
borderInfo:     Zeiger auf ein initialisiertes, leeres ArchivInfo

In borderInfo werden vier Bilder als glArchivItem_Bitmap_RLE an Index 0 bis 3 geschrieben, das sind die Rahmen oben, unten, links und rechts, wobei die Ecken zu oben/unten gehören. Sie müssen also an den Stellen
oben:   0        0
unten:  0        height-12
links:  0        12
rechts: widht-12 12
zu sehen sein.

Vor dem Aufruf von buildBorder() muss der interne, öffentliche Zeiger *palette auf ein ArchivItem_Palette* gesetzt werden.
 */

#include "defines.h" // IWYU pragma: keep
#include "customborderbuilder.h"
#include "ogl/glArchivItem_Bitmap_RLE.h"
#include "libsiedler2/src/ArchivInfo.h"

CustomBorderBuilder::CustomBorderBuilder(const libsiedler2::ArchivItem_Palette* const palette) : palette(palette)
{
    edgesLoaded = false;
    std::fill(edgesTop.begin(), edgesTop.end(), static_cast<BdrBitmap*>(NULL));
    std::fill(edgesBottom.begin(), edgesBottom.end(), static_cast<BdrBitmap*>(NULL));
    std::fill(edgesLeft.begin(), edgesLeft.end(), static_cast<BdrBitmap*>(NULL));
    std::fill(edgesRight.begin(), edgesRight.end(), static_cast<BdrBitmap*>(NULL));
    std::fill(fillersTop.begin(), fillersTop.end(), static_cast<BdrBitmap*>(NULL));
    std::fill(fillersBottom.begin(), fillersBottom.end(), static_cast<BdrBitmap*>(NULL));
    std::fill(fillersLeft.begin(), fillersLeft.end(), static_cast<BdrBitmap*>(NULL));
    std::fill(fillersRight.begin(), fillersRight.end(), static_cast<BdrBitmap*>(NULL));
}
CustomBorderBuilder::~CustomBorderBuilder()
{
    if (edgesLoaded)
    {
        // temporäre Bitmaps löschen
        for(unsigned char i = 0; i < numCorners; i++)
            delete corners[i];
        for(unsigned char i = 0; i < 3; i++)
        {
            delete edgesTop[i];
            delete edgesBottom[i];
            delete edgesLeft[i];
            delete edgesRight[i];
        }
        for(unsigned char i = 0; i < numFillersTop; i++)
            delete fillersTop[i];
        for(unsigned char i = 0; i < numFillersBottom; i++)
            delete fillersBottom[i];
        for(unsigned char i = 0; i < numFillersLeft; i++)
            delete fillersLeft[i];
        for(unsigned char i = 0; i < numFillersRight; i++)
            delete fillersRight[i];
    }
}

int CustomBorderBuilder::loadEdges(const libsiedler2::ArchivInfo* archiveInfo)
{
    // simples Fehlerabfangen
    if (archiveInfo->size() != 57)
        return 1; // nicht RESOURCE.DAT übergeben

    // Musterstücke einladen
    /* Evtl. könnte man hier nur den größten Rahmen laden und alle Teile aus diesem rauskopieren, um das Ganze etwas schneller zu machen.
       Allerdings sind die einander entsprechenden Stücke teilweise nicht in jeder Auflösung tatsächlich gleich, sodass man das für jedes vorher prüfen müsste.*/
    BdrBitmap* tempBMP = new BdrBitmap(1280, 1024);
    //palette = dynamic_cast<glArchivItem_Bitmap_RLE*>(archiveInfo->get(4))->getPalette();
    // 640x480
    {
        BitmapRLE2BdrBitmap(dynamic_cast<const glArchivItem_Bitmap_RLE*>(archiveInfo->get(4)), tempBMP);
        corners[0] = tempBMP->get(0, 0, 584, 12);
        corners[1] = tempBMP->get(584, 0, 56, 12);
        corners[2] = tempBMP->get(0, 468, 202, 12);
        corners[3] = tempBMP->get(508, 468, 132, 12);
        corners[4] = tempBMP->get(202, 468, 306, 12);
        corners[5] = tempBMP->get(0, 12, 12, 352);
        corners[6] = tempBMP->get(0, 364, 12, 104);
        corners[7] = tempBMP->get(628, 12, 12, 268);
        corners[8] = tempBMP->get(628, 280, 12, 188);
        fillersTop[0] = tempBMP->get(270, 0, 2, 12);
        fillersTop[2] = tempBMP->get(446, 0, 42, 12);
        fillersTop[3] = tempBMP->get(166, 0, 104, 12);
        fillersBottom[0] = tempBMP->get(508, 468, 2, 12);
    }
    // 800x600
    {
        BitmapRLE2BdrBitmap(dynamic_cast<const glArchivItem_Bitmap_RLE*>(archiveInfo->get(7)), tempBMP);
        edgesTop[0] = tempBMP->get(584, 0, 160, 12);
        edgesBottom[0] = tempBMP->get(508, 588, 160, 12);
        edgesLeft[0] = tempBMP->get(0, 364, 12, 120);
        edgesRight[0] = tempBMP->get(788, 280, 12, 120);
        fillersTop[1] = tempBMP->get(583, 0, 32, 12);
    }
    // 1024x768
    {
        BitmapRLE2BdrBitmap(dynamic_cast<const glArchivItem_Bitmap_RLE*>(archiveInfo->get(10)), tempBMP);
        edgesTop[1] = tempBMP->get(644, 0, 224, 12);
        edgesBottom[1] = tempBMP->get(668, 756, 224, 12);
        edgesLeft[1] = tempBMP->get(0, 484, 12, 168);
        edgesRight[1] = tempBMP->get(1012, 400, 12, 168);
        fillersBottom[1] = tempBMP->get(637, 756, 23, 12);
        fillersBottom[2] = tempBMP->get(601, 756, 34, 12);
        fillersBottom[3] = tempBMP->get(103, 756, 42, 12);
        fillersBottom[4] = tempBMP->get(242, 756, 72, 12);
        fillersLeft[0] = tempBMP->get(0, 524, 12, 2);
        fillersLeft[2] = tempBMP->get(0, 222, 12, 17);
        fillersLeft[3] = tempBMP->get(0, 423, 12, 30);
        fillersLeft[4] = tempBMP->get(0, 114, 12, 82);
        fillersRight[0] = tempBMP->get(1012, 280, 12, 2);
        fillersRight[1] = tempBMP->get(1012, 281, 12, 8);
        fillersRight[2] = tempBMP->get(1012, 528, 12, 15);
        fillersRight[3] = tempBMP->get(1012, 291, 12, 29);
        fillersRight[4] = tempBMP->get(1012, 475, 12, 54);
        fillersRight[5] = tempBMP->get(1012, 320, 12, 78);
    }
    // 1280x1024 links
    {
        BdrBitmap* pic1 = new BdrBitmap(640, 1024);
        BitmapRLE2BdrBitmap(dynamic_cast<const glArchivItem_Bitmap_RLE*>(archiveInfo->get(13)), pic1);
        tempBMP->put(0, 0, pic1);
        delete pic1;
    }
    // und rechts
    {
        BdrBitmap* pic2 = new BdrBitmap(640, 1024);
        BitmapRLE2BdrBitmap(dynamic_cast<const glArchivItem_Bitmap_RLE*>(archiveInfo->get(14)), pic2);
        tempBMP->put(640, 0, pic2);
        delete pic2;
        edgesTop[2] = tempBMP->get(968, 0, 256, 12);
        edgesBottom[2] = tempBMP->get(892, 1012, 256, 12);
        edgesLeft[2] = tempBMP->get(0, 652, 12, 256);
        edgesRight[2] = tempBMP->get(1268, 568, 12, 256);
        fillersLeft[1] = tempBMP->get(0, 769, 12, 9);
    }
    delete tempBMP;

    edgesLoaded = true;
    return 0;
}

int CustomBorderBuilder::buildBorder(const unsigned width, const unsigned height, boost::array<glArchivItem_Bitmap*, 4>& borderInfo)
{
    // simples Fehlerabfangen
    if (width < 640 || height < 480)
        return 1; // kleiner geht nicht
    if (!edgesLoaded)
        return 2; // Die Stücken sind noch nicht geladen worden, so gehts nicht!

    // temporäre BdrBitmap's deklarieren
    boost::array<BdrBitmap*, 4> customEdge;
    customEdge[0] = new BdrBitmap(width, 12); //oben
    customEdge[1] = new BdrBitmap(width, 12); //unten
    customEdge[2] = new BdrBitmap(12, height - 24); //links
    customEdge[3] = new BdrBitmap(12, height - 24); //rechts


    // den Rahmen zusammenbauen
    {
        // Ecken werden einfach eingefügt
        // horizontale Ecken:
        customEdge[0]->put(0, 0, corners[0]);
        customEdge[0]->put(width - 56, 0, corners[1]);
        customEdge[1]->put(0, 0, corners[2]);
        customEdge[1]->put(width - 132, 0, corners[3]);
        // das Mittelstück, damit das Bedienfeld passt
        customEdge[1]->put(width / 2 - 118, 0, corners[4]);
        // vertikale Ecken:
        customEdge[2]->put(0, 0, corners[5]);
        customEdge[2]->put(0, height - 128, corners[6]);
        customEdge[3]->put(0, 0, corners[7]);
        customEdge[3]->put(0, height - 212, corners[8]);

        // Freie Flächen mit Kanten ausfüllen
        // Kanten
        unsigned emptyFromPixel;
        unsigned toFillPixel;
        boost::array<unsigned char, 3> countEdge = {{0, 0, 0}};
        boost::array<unsigned short, 3> lengthEdge;
        // obere Kante
        emptyFromPixel = 584;
        toFillPixel = width - 640;
        for(unsigned char i = 0; i < 3; i++)
            lengthEdge[i] = edgesTop[i]->w;
        FindEdgeDistribution(toFillPixel, lengthEdge, countEdge);
        WriteEdgeDistribution(emptyFromPixel, 0, toFillPixel, false,
                              lengthEdge, countEdge, edgesTop, fillersTop, customEdge[0]);
        // untere Kante links
        emptyFromPixel = 202;
        toFillPixel = width / 2 - 320;
        for(unsigned char i = 0; i < 3; i++)
            lengthEdge[i] = edgesBottom[i]->w;
        FindEdgeDistribution(toFillPixel, lengthEdge, countEdge);
        WriteEdgeDistribution(emptyFromPixel, 0, toFillPixel, false,
                              lengthEdge, countEdge, edgesTop, fillersBottom, customEdge[1]);
        // untere Kante rechts
        emptyFromPixel = width / 2 + 188;
        toFillPixel = width - width / 2 - 320; // hier steht w - w/2 statt w/2, um den Rundungsfehler bei ungeraden w zu kompensieren
        for(unsigned char i = 0; i < 3; i++)
            lengthEdge[i] = edgesBottom[i]->w;
        FindEdgeDistribution(toFillPixel, lengthEdge, countEdge);
        WriteEdgeDistribution(emptyFromPixel, 0, toFillPixel, false,
                              lengthEdge, countEdge, edgesTop, fillersBottom, customEdge[1]);
        // linke Kante
        emptyFromPixel = 352;
        toFillPixel = height - 480;
        for(unsigned char i = 0; i < 3; i++)
            lengthEdge[i] = edgesLeft[i]->h;
        FindEdgeDistribution(toFillPixel, lengthEdge, countEdge);
        WriteEdgeDistribution(0, emptyFromPixel, toFillPixel, true,
                              lengthEdge, countEdge, edgesLeft, fillersLeft, customEdge[2]);
        // rechte Kante
        emptyFromPixel = 268;
        toFillPixel = height - 480;
        for(unsigned char i = 0; i < 3; i++)
            lengthEdge[i] = edgesRight[i]->h;
        FindEdgeDistribution(toFillPixel, lengthEdge, countEdge);
        WriteEdgeDistribution(0, emptyFromPixel, toFillPixel, true,
                              lengthEdge, countEdge, edgesRight, fillersRight, customEdge[3]);
    }

    // Bildspeicher für Ausgaberahmen vorbereiten; in glArchivItem_Bitmap_RLE kovertieren
    for(unsigned i = 0; i < 4; i++)
    {
        glArchivItem_Bitmap_RLE* customEdgeRLE = new glArchivItem_Bitmap_RLE;
        customEdgeRLE->init(customEdge[i]->w, customEdge[i]->h, libsiedler2::FORMAT_PALETTED, palette);
        BdrBitmap2BitmapRLE2(customEdge[i], customEdgeRLE);
        borderInfo[i] = customEdgeRLE;
    }
    // Speicher der BdrBitmap's wieder freigeben
    for(unsigned char i = 0; i < 4; i++)
        delete customEdge[i];

    return 0;
}


void CustomBorderBuilder::BitmapRLE2BdrBitmap(const glArchivItem_Bitmap_RLE* bitmapRLE, BdrBitmap* bdrBitmap)
{
    unsigned short x, y;
    for(y = 0; y < bitmapRLE->getHeight(); y++)
        for(x = 0; x < bitmapRLE->getWidth(); x++)
        {
            bdrBitmap->put(x, y, bitmapRLE->getPixelClrIdx(x, y));
        }
}


void CustomBorderBuilder::BdrBitmap2BitmapRLE2(BdrBitmap* bdrBitmap, glArchivItem_Bitmap_RLE* bitmapRLE)
{
    for(unsigned y = 0; y < bdrBitmap->h; y++)
        for(unsigned x = 0; x < bdrBitmap->w; x++)
        {
            bitmapRLE->setPixel(x, y, bdrBitmap->get(x, y));
        }
}


void CustomBorderBuilder::FindEdgeDistribution(unsigned toFill, boost::array<unsigned short, 3>& lengths, boost::array<unsigned char, 3>& shouldCounts)
{
    // Die should-Variablen speichern die bisher als am besten befundene Kombination; die would-Variablen die gerade zu prüfende
    shouldCounts[0] = 0; shouldCounts[1] = 0; shouldCounts[2] = 0;
    unsigned char wouldCounts[3];
    unsigned char maxCounts[3]; // wieviel mal passt jedes Teil maximal in die Freifläche?
    for(unsigned char i = 0; i < 3; i++)
        maxCounts[i] = toFill / lengths[i];
    unsigned shouldBeFilled = 0;
    unsigned char shouldNumDifferentTiles = 0;
    // Schleife über alle möglichen Kombinationen
    for(wouldCounts[0] = 0; wouldCounts[0] <= maxCounts[0]; wouldCounts[0]++)
        for(wouldCounts[1] = 0; wouldCounts[1] <= maxCounts[1]; wouldCounts[1]++)
            for(wouldCounts[2] = 0; wouldCounts[2] <= maxCounts[2]; wouldCounts[2]++)
            {
                // Finde, wieviel Platz die Kombination ausfüllen würde
                unsigned wouldBeFilled = 0;
                for(unsigned char i = 0; i < 3; i++)
                    wouldBeFilled += wouldCounts[i] * lengths[i];
                // wenn die Kombination nicht zu groß ist und weniger oder gleich viel Platz frei ließe als/wie bisher
                if (wouldBeFilled <= toFill) if (wouldBeFilled >= shouldBeFilled)
                    {
                        // Finde, ob mehr verschiedene Stücken benutzt würden als bisher
                        unsigned char wouldNumDifferentTiles = 3;
                        for(unsigned char i = 0; i < 3; i++) if (wouldCounts[i] == 0)
                                wouldNumDifferentTiles--;
                        // wenn mehr Stücke benutzt würden oder weniger Freifläche bleibt
                        if ((wouldNumDifferentTiles > shouldNumDifferentTiles) || (wouldBeFilled > shouldBeFilled))
                        {
                            // Bessere Verteilung gefunden, in should-Variablen speichern
                            for(unsigned char i = 0; i < 3; i++)
                                shouldCounts[i] = wouldCounts[i];
                            shouldNumDifferentTiles = wouldNumDifferentTiles;
                            shouldBeFilled = wouldBeFilled;
                        }
                    }
            }
}


template<size_t T_numEdges, size_t T_numFillers>
void CustomBorderBuilder::WriteEdgeDistribution(const unsigned x,
        const unsigned y,
        const unsigned toFill,
        const bool direction, // false = waagerecht, true = senkrecht
        const boost::array<unsigned short, 3>& edgeLengths,
        boost::array<unsigned char, 3>& edgeCounts,
        boost::array<BdrBitmap*, T_numEdges> edges,
        boost::array<BdrBitmap*, T_numFillers> fillers,
        BdrBitmap* outBorder)
// Schreibt die übergebene Verteilung ins Bild und füllt die restliche Fläche auf.
{
    unsigned emptyFromPixel = direction ? y : x;
    unsigned toFillPixel = toFill;

    // Wieviele große Stücken zu schreiben?
    unsigned char edgesToDistribute = 0;
    for(unsigned char i = 0; i < 3; i++)
        edgesToDistribute += edgeCounts[i];

    // Der Reihe nach schreiben
    unsigned takeEdgeNum = 0;
    while (edgesToDistribute > 0)
    {
        if (edgeCounts[takeEdgeNum] > 0)
        {
            if (direction)
                outBorder->put(x, emptyFromPixel, edges[takeEdgeNum]);
            else
                outBorder->put(emptyFromPixel, y, edges[takeEdgeNum]);
            emptyFromPixel += edgeLengths[takeEdgeNum];
            toFillPixel -= edgeLengths[takeEdgeNum];
            edgeCounts[takeEdgeNum]--;
            edgesToDistribute--;
        }
        takeEdgeNum = (takeEdgeNum + 1) % 3;
    }

    // Fülle den Rest auf
    boost::array<unsigned char, T_numFillers> numFillersToUse;
    std::fill(numFillersToUse.begin(), numFillersToUse.end(), 0);
    // Finde, wie oft jedes Füllerstück gebraucht wird. Einfach von groß nach klein einfügen, wenn der Platz jeweils noch reicht.
    unsigned char curFiller = T_numFillers - 1;
    while (toFillPixel > 0)
    {
        unsigned short length = direction ? fillers[curFiller]->h : fillers[curFiller]->w;
        if (length <= toFillPixel)
        {
            numFillersToUse[curFiller]++;
            toFillPixel -= length;
        }
        else
        {
            if (curFiller > 0) // Das nächstkleinere Füllerstück testen
                curFiller--;
            else // Wenn am Ende weniger Platz freibleibt als das kleinste Füllerstück groß ist (2 Pixel), wird sofort ein passendes Teilstück eingefügt.
            {
                if (direction)
                    outBorder->put(x, emptyFromPixel, fillers[0]->get(0, 0, 12, toFillPixel), true);
                else
                    outBorder->put(emptyFromPixel, y, fillers[0]->get(0, 0, toFillPixel, 12), true);
                emptyFromPixel += toFillPixel;
                toFillPixel = 0;
            }
        }
    }

    // Schreibe die Füllerstückkombination von groß nach klein
    unsigned char numUsedFillers = 0;
    for(unsigned char i = 1; i < T_numFillers; i++)
        numUsedFillers += numFillersToUse[i];
    for(unsigned char i = T_numFillers - 1; (i >= 1); i--) //
    {
        for(unsigned char j = 0; j < numFillersToUse[i]; j++)
        {
            // Vor jedem großen zuerst ein paar der kleinsten Füller, damit die großen nicht so aneinander gequetscht sind.
            for(unsigned char j = 0; j < numFillersToUse[0] / (numUsedFillers + 1); j++)
            {
                if (direction)
                    outBorder->put(x, emptyFromPixel, fillers[0]);
                else
                    outBorder->put(emptyFromPixel, y, fillers[0]);
                emptyFromPixel += direction ? fillers[0]->h : fillers[0]->w;
            }
            if (direction)
                outBorder->put(x, emptyFromPixel, fillers[i]);
            else
                outBorder->put(emptyFromPixel, y, fillers[i]);
            emptyFromPixel += direction ? fillers[i]->h : fillers[i]->w;
        }
    }
    // Die restlichen kleinen Füller
    const unsigned char numSmallFillers = numFillersToUse[0] - (numFillersToUse[0] / (numUsedFillers + 1)) * numUsedFillers;
    for(unsigned char j = 0; j < numSmallFillers; j++)
    {
        if (direction)
            outBorder->put(x, emptyFromPixel, fillers[0]);
        else
            outBorder->put(emptyFromPixel, y, fillers[0]);
        emptyFromPixel += direction ? fillers[0]->h : fillers[0]->w;
    }
}


/********************************************* CustomBorderBuilder-Klasse ---*/

/*--- BdrBitmap-Klasse *******************************************************/
// public
CustomBorderBuilder::BdrBitmap::BdrBitmap(const unsigned width, const unsigned height)
{
    w = width;
    h = height;
    values.resize(w*h);
}

CustomBorderBuilder::BdrBitmap* CustomBorderBuilder::BdrBitmap::get(const unsigned x, const unsigned y, const unsigned width, const unsigned height) const
{
    BdrBitmap* pic = new BdrBitmap(width, height);
    for(unsigned i = 0; i < pic->h; i++)
        for(unsigned j = 0; j < pic->w; j++)
        {
            pic->values[pic->getpos(j, i)] = values[getpos(x + j, y + i)];
        }
    return pic;
}

inline unsigned char CustomBorderBuilder::BdrBitmap::get(const unsigned x, const unsigned y) const
{
    return values[getpos(x, y)];
}

void CustomBorderBuilder::BdrBitmap::put(const unsigned x, const unsigned y, BdrBitmap* pic, bool picGetted)
{
    for(unsigned i = 0; i < pic->h; i++)
        for(unsigned j = 0; j < pic->w; j++)
        {
            values[getpos(x + j, y + i)] = pic->values[pic->getpos(j, i)];
        }
    if (picGetted)
        delete pic;
}

inline void CustomBorderBuilder::BdrBitmap::put(const unsigned x, const unsigned y, const unsigned char c)
{
    values[getpos(x, y)] = c;
}

// private

inline const unsigned CustomBorderBuilder::BdrBitmap::getpos(const unsigned x, const unsigned y) const
// liefert den Index eines Pixels (x,y) im internen Speicher value[]
{
    return w * y + x;
}

/******************************************************* BdrBitmap-Klasse ---*/

