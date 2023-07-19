// export class Song {
//     public path: string;
//     public title: string;
//     public time: number;
//     //--------- TODO ---------
//     constructor(path: string | null) {
//         if (path === null) path='';
//         this.path = path;
//         this.title = path;
//         this.time = 0;
//     }
// }
// export let playlist = new LinkedList();
// export let curSong = new Song(null);

// let curNode: Node | null;

// export function setNode(node: Node){
//     curNode = node;
//     curSong = node.getElement();
//     console.log(curSong);
// }

// export function setSong(song: Song){
//     if(curSong !== song){
//         //curSec = 0;
//     }
//     curSong = song;
// }

// // Add Song
// export function addSong(filePath: string){
//     let song = new Song(filePath);
//     if(curSong===null){
//         curSong=song
//         curNode = playlist.getHead();
//     };
//     playlist.append(song);
// }
// // Remove Song
// export function removeSong(pos: number){
//     let oldNode = playlist.removeAt(pos);
//     if (curNode === oldNode){
//         if(oldNode.getNext() !== null){
//             curNode = oldNode.getNext();
//         }
//         else{
//             curNode = playlist.getHead();
//         }
//         if(curNode !== null) setSong(curNode.getElement());
//     }
// }
// export function addPlayList(filePath: string){
//     //For song
//     addSong(filePath);
// }
// export function clearPlayList(){
//     // JS will free out of scope memory
//     playlist = new LinkedList();
//     curSong = new Song(null);
//     curNode = null;
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////
// //-------------------------------------------- DEV --------------------------------------------//
// /////////////////////////////////////////////////////////////////////////////////////////////////

// let song1 = new Song('C:Desktop\songs\Montero (call me by your name) by Lil Nas X');
// song1.title = 'Montero (call me by your name) by Lil Nas X';
// song1.time = 299;
// let song2 = new Song('C:Desktop\songs\Head & Heart by Joel Corry ft Mnek');
// song2.title = 'Head & Heart by Joel Corry ft Mnek';
// song2.time = 158;
// let song3 = new Song('C:Desktop\songs\You broke me first by Tate Mcrae');
// song3.title = 'You broke me first by Tate Mcrae';
// song3.time = 200;
// let song4 = new Song('C:Desktop\songs\Lean On by Major Lazer & Dj shake ft. Mo');
// song4.title = 'Lean On by Major Lazer & Dj shake ft. Mo';
// song4.time = 174;
// playlist.append(song1);
// playlist.append(song2);
// playlist.append(song3);
// playlist.append(song4);