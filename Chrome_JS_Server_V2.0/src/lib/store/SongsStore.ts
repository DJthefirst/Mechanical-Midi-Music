import { writable } from 'svelte/store';

export interface SongElement extends File{
    title: string;
}
export const SongsStore = writable<SongElement[]>([]);


export const addSong = (newSong: SongElement) => {
    SongsStore.update( (currentSongs) => [...currentSongs, newSong] )   
}