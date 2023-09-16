<script lang="ts">
	import { Playlist, Node, Song } from './Playlist';
	import { onMount } from 'svelte';
	import JZZ from 'jzz'; // @ts-ignore
	import SMF from 'jzz-midi-smf';
	import { midiNodeStore } from '$lib/store/stores';
	SMF(JZZ);

	$: playlist = new Playlist();

	// let testSong1: any = '/short.mid';

	$: curSong = new Song(null);
	let curNode: Node | null;
	let player: any = null;

	//State Variables
	let doLoop = false;
	let doPlay = false;

	let curMS = 0;
	$: curTime = secondsToTime(curMS / 1000); //seconds

	function updateCurTime() {
		if (!doPlay) return;
		curMS = player.positionMS();
		setTimeout(updateCurTime, 200);
	}

	function togglePlay() {
		doPlay = !doPlay;
		if (player == null) return;

		if (doPlay) player.resume();
		else player.pause();
		updateCurTime();
	}

	$: curSong, updateSong();
	async function updateSong() {
		if (curSong.title == '') return; // @ts-ignore
		let songFile = JZZ.MIDI.SMF(await curSong.getData())

		if (player != null) player.stop();
		player = songFile.player();
		player.connect($midiNodeStore);
		player.onEnd = function () {
			setTimeout(nextSong, 500);
		};
		if (doPlay) player.play();
	}

	function setNode(node: Node) {
		curMS = 0;
		curNode = node;
		curSong = node.getElement();
		console.log(curSong);
	}

	function setSong(song: Song) {
		if (curSong !== song) {
			curMS = 0;
		}
		curSong = song;
	}

	function nextSong() {
		if (curNode === null) return;
		if (doLoop) {
			curSong = curSong;
			return;
		}
		let node = curNode.getNext();
		if (node === null) node = playlist.getHead();
		setNode(node);
	}

	// Add Song
	export async function addSong(file: File) {
		let song = new Song(file); // @ts-ignore
		song.time = Math.floor(JZZ.MIDI.SMF(await song.getData()).player().durationMS() / 1000);
		if (curSong === null) {
			curSong = song;
			curNode = playlist.getHead();
		}
		playlist.append(song);
		playlist = playlist;
	}

	// Remove Song
	export function removeSong() {
		let i = 0;
		for (let node of playlist) {
			if (node == curNode) break;
			i++;
		}
		console.log(i);

		let oldNode = playlist.removeAt(i);
		if (curNode === oldNode) {
			if (oldNode.getNext() !== null) {
				curNode = oldNode.getNext();
			} else {
				curNode = playlist.getHead();
			}
			if (curNode !== null) setSong(curNode.getElement());
		}
		curSong = curSong;
	}

	export function clearPlayList() {
		// JS will free out of scope memory
		playlist = new Playlist();
		curSong = new Song(null);
		curMS = 0;
		player.stop();
		player = null;
		curNode = null;
	}

	function stop() {
		doPlay = false;
		curMS = 0;
		if (player == null) return;
		player.stop();
	}

	function secondsToTime(len: number) {
		let min = String(Math.floor(len / 60));
		let sec = String(Math.floor(len % 60));
		if (sec.length < 2) sec = '0' + sec;
		return min + ':' + sec;
	}
</script>

<div class="w-full bg-gray-dark rounded-md">
	<ol class="w-full h-screen max-h-[calc(16rem)] p-2 list-decimal overflow-y-scroll">
		{#if curSong !== null}
			{#each playlist as node}
				<!-- svelte-ignore a11y-no-static-element-interactions -->
				<div
					class="li-player-song {curSong == node.getElement()
						? 'bg-gray-select font-semibold'
						: 'hover:bg-gray-700'}"
					on:click={() => setNode(node)}
				>
					<li class="pl-2">{node.getElement().title.slice(0,25)}</li>
					<span class=".self-end pl-4 font-semibold">{secondsToTime(node.getElement().time)}</span>
				</div>
			{/each}
		{/if}
	</ol>
	<div class="w-full p-0 m-0">
		<input
			class="slider-player w-full cursor-pointer"
			type="range"
			min="0"
			max={curSong.time * 1000}
			on:click={player != null && player.jumpMS(this.value)}
			bind:value={curMS}
		/>
	</div>
	<div class="flex flex-row justify-between px-2 pb-3 pt-1">
		<span>-{curTime}</span>

		<!-- svelte-ignore a11y-no-static-element-interactions -->
		<i
			class="material-icons {doLoop ? 'bg-gray-select' : 'bg-gray-dark hover:bg-gray-700'} 
            rounded-full cursor-pointer select-none p-1 my-1"
			on:click={() => (doLoop = !doLoop)}>loop</i
		>

		<!-- svelte-ignore a11y-no-static-element-interactions -->
		<i
			class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none p-1 my-1"
			on:click={() => (curMS > 5000 ? player.jumpMS((curMS -= 5000)) : null)}>fast_rewind</i
		>

		<!-- svelte-ignore a11y-no-static-element-interactions -->
		<i
			class="material-icons bg-gray-select rounded-full cursor-pointer select-none p-2"
			on:click={() => togglePlay()}>{doPlay ? 'pause' : 'play_arrow'}</i
		>

		<!-- svelte-ignore a11y-no-static-element-interactions -->
		<i
			class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none p-1 my-1"
			on:click={() => stop()}>stop</i
		>

		<!-- svelte-ignore a11y-no-static-element-interactions -->
		<i
			class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none p-1 my-1"
			on:click={() => (curMS < curSong.time * 1000 - 5000 ? player.jumpMS((curMS += 5000)) : null)}
			>fast_forward</i
		>

		<!-- svelte-ignore a11y-no-static-element-interactions -->
		<i
			class="material-icons hover:bg-gray-700 rounded-full cursor-pointer select-none p-1 my-1"
			on:click={() => nextSong()}>shortcut</i
		>

		<span>{secondsToTime(curSong.time)}</span>
	</div>
</div>
