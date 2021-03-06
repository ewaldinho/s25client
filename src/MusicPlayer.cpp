// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
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

#include "rttrDefines.h" // IWYU pragma: keep
#include "MusicPlayer.h"
#include "Loader.h"
#include "drivers/AudioDriverWrapper.h"
#include "ingameWindows/iwMusicPlayer.h"
#include "ogl/MusicItem.h"
#include "libsiedler2/Archiv.h"
#include "libsiedler2/ArchivItem.h"
#include "libsiedler2/prototypen.h"
#include "libutil/Log.h"
#include "libutil/StringConversion.h"
#include <boost/nowide/fstream.hpp>
#include <algorithm>
#include <sstream>

Playlist::Playlist() : current(-1), repeats(1), random(false) {}

/**
 *  startet das Abspielen der Playlist.
 */
void Playlist::Prepare()
{
    if(!songs.empty())
    {
        order.clear();

        // Abspielreihenfolge erstmal normal festlegen
        order.resize(songs.size() * repeats);

        // normale Reihenfolge
        for(unsigned i = 0; i < songs.size() * repeats; ++i)
            order[i] = i % songs.size();

        // Bei Zufall nochmal mischen
        if(random)
            std::random_shuffle(order.begin(), order.end());
    }
}

/**
 *  Playlist in Datei speichern
 */
bool Playlist::SaveAs(const std::string& filename, const bool overwrite)
{
    if(!overwrite)
    {
        bnw::ifstream in(filename.c_str());
        if(in.good())
        {
            // Datei existiert und wir sollen sie nicht überschreiben
            in.close();
            return false;
        }
    }

    bnw::ofstream out(filename.c_str());
    if(!out.good())
        return false;

    out << repeats << " ";
    out << (random ? "random" : "ordered") << std::endl;

    // songs reinschreiben
    for(unsigned i = 0; i < songs.size(); ++i)
        out << songs[i] << "\n";

    out.close();

    return true;
}

/**
 *  Playlist laden
 */
bool Playlist::Load(const std::string& filename)
{
    if(filename.length() == 0)
        return false;

    LOG.write("lade \"%s\"\n") % filename;

    bnw::ifstream in(filename.c_str());

    if(in.fail())
        return false;

    std::string line, random_str;
    std::stringstream sline;

    if(!std::getline(in, line))
        return false;
    printf("%s\n", line.c_str());
    sline.clear();
    sline << line;
    sline >> repeats;
    sline >> random_str;

    random = (random_str == "random_playback" || random_str == "random");

    // Liste leeren von evtl vorherigen Stücken
    /*songs.clear();
    order.clear();*/

    while(true)
    {
        std::getline(in, line);

        // Carriage returns & Line feeds aus der Zeile ggf. rausfiltern
        std::string::size_type k = 0;
        while((k = line.find('\r', k)) != std::string::npos)
            line.erase(k, 1);
        k = 0;
        while((k = line.find('\n', k)) != std::string::npos)
            line.erase(k, 1);

        // bei leeren Zeilen oder bei eof aufhören
        if(line.length() == 0 || in.eof())
        {
            in.close();

            // geladen, also zum Abspielen vorbereiten
            Prepare();

            return true;
        }

        songs.push_back(line);
    }

    return false;
}

/**
 *  Füllt das iwMusicPlayer-Fenster mit den entsprechenden Werten
 */
void Playlist::FillMusicPlayer(iwMusicPlayer* window) const
{
    window->SetSegments(songs);
    window->SetRepeats(repeats);
    window->SetRandomPlayback(random);

    if(current >= 0)
        window->SetCurrentSong(current);
}

/**
 *  Liest die Werte aus dem iwMusicPlayer-Fenster
 */
void Playlist::ReadMusicPlayer(const iwMusicPlayer* const window)
{
    repeats = window->GetRepeats();
    random = window->GetRandomPlayback();
    songs = window->GetSegments();

    // zum Abspielen vorbereiten
    Prepare();
}

// Wählt den Start-Song aus
void Playlist::SetStartSong(const unsigned id)
{
    for(unsigned i = 0; i < order.size(); ++i)
    {
        if(order[i] == id)
        {
            std::swap(order[0], order[i]);
            return;
        }
    }
}

MusicPlayer::MusicPlayer() : playing(false) {}

/**
 *  Startet Abspielvorgang
 */
void MusicPlayer::Play()
{
    playing = true;

    PlayNext();
}

/**
 *  Stoppt Abspielvorgang
 */
void MusicPlayer::Stop()
{
    AUDIODRIVER.StopMusic();
    playing = false;
}

/**
 * Spielt nächstes Stück ab
 */
void MusicPlayer::PlayNext()
{
    std::string song = list.getNextSong();

    // Am Ende der Liste angekommen?
    if(song.empty())
    {
        Stop();
        return;
    }

    // Evtl ein Siedlerstück ("sNN")?
    if(song.length() == 3 && song[0] == 's')
    {
        unsigned nr = s25util::fromStringClassicDef(song.substr(1), 999u);
        if(nr <= 14)
        {
            // Siedlerstück abspielen (falls es geladen wurde)
            MusicItem* curSong = dynamic_cast<MusicItem*>(LOADER.sng_lst[nr - 1]);
            if(curSong)
                curSong->Play(1);
        }
        return;
    }

    // anderes benutzerdefiniertes Stück abspielen
    // in "sng" speichern, daher evtl. altes Stück erstmal löschen
    sng.clear();

    LOG.write("lade \"%s\": ") % song;

    // Neues Stück laden
    if(libsiedler2::loader::LoadSND(song, sng) != 0)
    {
        Stop();
        return;
    }

    // Und abspielen
    MusicItem* curSong = dynamic_cast<MusicItem*>(sng[0]);
    if(curSong)
        curSong->Play(1);
}

/// schaltet einen Song weiter und liefert den Dateinamen des aktuellen Songs
const std::string Playlist::getNextSong()
{
    const std::string tmp(getCurrentSong());
    current = !songs.empty() && !order.empty() ? static_cast<int>(order[0]) : -1;
    if(!order.empty())
        order.erase(order.begin());
    return tmp;
}
