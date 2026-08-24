const panel_width = 1000;
const panel_height = 75;



window.addEventListener("load", makePhaseTwo);

// Lotsa magic numbers below...

function makePhaseTwo() {
    const svg = d3.select("body")
        .append("svg")
        .attr("width", panel_width)
        .attr("height", panel_height);

    svg.append("rect")
        .attr("x", 20)
        .attr("y", 20)
        .attr("width", 450)
        .attr("height", 50)
        .attr("fill", "lightgreen")
        .attr("stroke", "black")
        .attr("rx", 10)
        .attr("ry", 10);

    svg.append("text")
        .attr("x", 235)  // Center text in the rectangle
        .attr("y", 50)
        .attr("text-anchor", "middle")  // Center horizontally
        .attr("dominant-baseline", "middle")  // Center vertically
        .attr("fill", "black")
        .attr("font-size", "20px")
        .attr("font-family", "Arial, sans-serif")
        .text("Languages: Paradigms | Creator, Year");
    
    d3.json("languages-simpler.json").then(data => {
        
            d3.select("body")
                .append("ul")
                .selectAll("li")
                .data(data.languages)
                .enter()
                .append("li")
                .style("font-size", "18px")
                .style("margin-bottom", "12px")
                .text( d => `${d.name}: ${d.paradigm.slice(0, 3).join(", ")}`)
                .on("click", function(event, d){
                    this.toggleState = !this.toggleState;
                    d3.select(this)
                        .text(this.toggleState?`${d.name}: ${d.paradigm.slice(0, 3).join(", ")} | ${d.creator}, ${d.year}`:`${d.name}: ${d.paradigm.slice(0, 3).join(", ")}`);
                });
    });
}
