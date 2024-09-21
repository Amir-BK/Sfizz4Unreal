# Sfizz4Unreal
A small plugin that allows using Sfizz as a Synthesizer within Unreal and Metasounds, deisgned for Unreal 5.4+ with Harmonix plugin

Uses and depends on Sfizz - https://github.com/sfztools/sfizz


Status: 
Right now only an SFZ synth component is provided, it can primitively receive note on and note off events and play them directly to the assigned submix (it works just like a regular audio component), I intend to also create a custom metasound node that receives the Harmonix MidiStream data type and generated audio inside a metasound, as an alternative to the fusion sampler. 
