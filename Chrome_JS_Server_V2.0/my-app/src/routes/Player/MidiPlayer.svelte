<script lang="ts">

    import { LinkedList, Node } from "./LinkedList";
    // import { Song } from './Playlist';
    // import {playlist, curSong} from './Playlist';
    // import {setNode, addSong, addPlayList, removeSong, clearPlayList} from './Playlist';

    class Song {
    public path: string;
    public title: string;
    public time: number;
    //--------- TODO ---------
    constructor(path: string | null) {
        if (path === null) path='';
        this.path = path;
        this.title = path;
        this.time = 0;
    }
}

    let playlist = new LinkedList();
    let curSong = new Song(null);

    let curNode: Node | null;

    //State Variables
    let doLoop = false;
    let doPlay = false;

    let curSec = 0;
    $: curTime = secondsToTime(curSec); //seconds

    /////////////////////////////////////////////////////////////////////////////////////////////////
    //-------------------------------------------- DEV --------------------------------------------//
    /////////////////////////////////////////////////////////////////////////////////////////////////

    let song1 = new Song('C:Desktop\songs\Montero (call me by your name) by Lil Nas X');
    song1.title = 'Montero (call me by your name) by Lil Nas X';
    song1.time = 299;
    let song2 = new Song('C:Desktop\songs\Head & Heart by Joel Corry ft Mnek');
    song2.title = 'Head & Heart by Joel Corry ft Mnek';
    song2.time = 158;
    let song3 = new Song('C:Desktop\songs\You broke me first by Tate Mcrae');
    song3.title = 'You broke me first by Tate Mcrae';
    song3.time = 200;
    let song4 = new Song('C:Desktop\songs\Lean On by Major Lazer & Dj shake ft. Mo');
    song4.title = 'Lean On by Major Lazer & Dj shake ft. Mo';
    song4.time = 174;
    playlist.append(song1);
    playlist.append(song2);
    playlist.append(song3);
    playlist.append(song4);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    //-------------------------------------------- DEV --------------------------------------------//
    /////////////////////////////////////////////////////////////////////////////////////////////////


    function setNode(node: Node){
        curSec = 0;
        curNode = node;
        curSong = node.getElement();
        console.log(curSong);
    }

    function setSong(song: Song){
        if(curSong !== song){
            curSec = 0;
        }
        curSong = song;
    }

    function nextSong(){
        if (curNode === null) return;
        let node = curNode.getNext();
        if (node === null) node = playlist.getHead();
        setNode(node);
    }

    // Add Song
    function addSong(filePath: string){
        let song = new Song(filePath);
        if(curSong===null){
            curSong=song
            curNode = playlist.getHead();
        };
        playlist.append(song);
    }

    // Remove Song
    function removeSong(pos: number){
        let oldNode = playlist.removeAt(pos);
        if (curNode === oldNode){
            if(oldNode.getNext() !== null){
                curNode = oldNode.getNext();
            }
            else{
                curNode = playlist.getHead();
            }
            if(curNode !== null) setSong(curNode.getElement());
        }
    }

    function addPlayList(filePath: string){
        //For song
        addSong(filePath);
    }

    function clearPlayList(){
        // JS will free out of scope memory
        playlist = new LinkedList();
        curSong = new Song(null);
        curNode = null;
    }

    function secondsToTime(len: number){
        let min = String(Math.floor(len/60));
        let sec = String(len%60)
        if (sec.length < 2) sec = '0' + sec;
        return(min + ':' + sec);
    }

</script>

<div class= 'w-full bg-gray-dark rounded-md'>
    <ol class= 'w-full h-screen max-h-[calc(16rem)] p-2 list-decimal overflow-y-scroll'>

        {#if curSong !== null}
        {#each playlist as node}
            {#if curSong == node.getElement()}
            <div class= 'li-player-song bg-blue-800' on:click={() => setNode(node) }>
                <li class= 'pl-2'>{node.getElement().title}</li>
                <span class='.self-end pl-4'>{secondsToTime(node.getElement().time)}</span>
            </div>
            {:else}
            <div class= 'li-player-song hover:bg-gray-700' on:click={() => setNode(node)}>
                <li class= 'pl-2'>{node.getElement().title}</li>
                <span class='.self-end pl-4'>{secondsToTime(node.getElement().time)}</span>
            </div>
            {/if}
        {/each}
        {/if}
 
    </ol>
    <div class= 'w-full p-0 m-0'>
        <input class= 'slider-player w-full cursor-pointer'
        type="range" min="0" max={curSong.time} bind:value={curSec}>
    </div>
    <div class= 'flex flex-row justify-between px-2 pb-3 pt-1' >
        <span>-{curTime}</span>
        <i class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none"
            on:click={() => doLoop= !doLoop}>loop</i>
        <i class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none"
            on:click={() => curSec = (curSec>10) ? curSec -= 10 : 0}>fast_rewind</i>
        <i class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none">play_circle</i>
        <i class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none">stop</i>
        <i class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none"
            on:click={() => curSec = (curSec<curSong.time-10) ? curSec += 10 : curSong.time}>fast_forward</i>
        <i class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none"
            on:click={() => nextSong()}>shortcut</i>
        <span>{secondsToTime(curSong.time)}</span>
    </div>
</div>