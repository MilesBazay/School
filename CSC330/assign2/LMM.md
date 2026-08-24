#### LMM notes

You are to maintain this markdown file to keep notes on the way large language models have helped with the completion of your A2 solution (e.g., ChatGPT, Copilot, )


##### Phase 1 notes
Question: after implementing the langauges json into phase 1 how do I actually print out another language do I need to create another svg.append with the langauge information and have that in a loop to run through the rest of the langauges?
Answer: svg.selectAll("text")
            .data(data.languages)
            .enter()
            To get a list of all the languages in the file
            .attr("y", function(d, i) {
                return 30 + i * 25;
            })
            .text(function(d) {
                return d.name + " - " + d.paradigm[0];
            });
            To adjust the Y value after each input and print the name and paradigm for each language
            
I used the implementation from the slides instead.
            
Question: Using python3 -m http.server -d ./phase-1 8080. My changes are not showing up on the localhost.
Answer: Try going inside phase-1 instead and using python3 -m http.server 8080.

Question: The display of languages gets cutoff after 100 pixels or so, how do I expand the panel
Answer: Change these variables:
    const panel_width = 300;
    const panel_height = 600;

##### Phase 2 notes

Question: How do I change this format to work inside my d3 section : d3.select("#bigger")
                                                                        .on("click", makeSelectExample);
                
Answer: .on("click", function(event, d) {

Question: Asked to take a look at a portion of my code for debugging

Answer: I had an extra semicolon before the end of my chain of .funcs



##### Phase 3 notes

Question: How do I make my highlight cut off at the end of the sentence instead fo going across the whole screen

Answer: try adding .style("width", "fit-content") to ur li

Question: How do I grab each language that is an influence for the langauge that I am hovering because right now what I tried is giving me the full list and I cannot individually attach styles to them

Answer: Try using 
    d3.selectAll("li")
        .filter(function(otherLanguage) {
            return d.influences.includes(otherLanguage.name);
        })
        
Question: I am trying to reset the shift on mouse out but It is moving it the opposite direction too far.

Answer: use a 0px transform instead of trying to move it back the other direction. 
