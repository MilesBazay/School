const panel_width = 500;
const panel_height = 75;



window.addEventListener("load", makePhaseThree);

// Lotsa magic numbers below...

function makePhaseThree() {
    const svg = d3.select("body")
        .append("svg")
        .attr("width", panel_width)
        .attr("height", panel_height);

    svg.append("rect")
        .attr("x", 20)
        .attr("y", 20)
        .attr("width", 450)
        .attr("height", 50)
        .attr("fill", "purple")
        .attr("stroke", "black")
        .attr("rx", 10)
        .attr("ry", 10);

    svg.append("text")
        .attr("x", 235)  // Center text in the rectangle
        .attr("y", 50)
        .attr("text-anchor", "middle")  // Center horizontally
        .attr("dominant-baseline", "middle")  // Center vertically
        .attr("fill", "white")
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
                .style("width", "fit-content")
                .text( d => `${d.name}: ${d.paradigm.slice(0, 3).join(", ")}`)
                .on("click", function(event, d){
                    this.toggleState = !this.toggleState;
                    d3.select(this)
                        .text(this.toggleState?`${d.name}: ${d.paradigm.slice(0, 3).join(", ")} | ${d.creator}, ${d.year}`:`${d.name}: ${d.paradigm.slice(0, 3).join(", ")}`);
                })
                .on("mouseenter", function(event, d){
                    d3.select(this)
                        .style("font-weight", "bold")
                        .style("background-color", "yellow");
                    d3.selectAll("li")
                        .filter(function(otherLanguage) {
                            return d.influences.includes(otherLanguage.name);
                        })
                        .style("transform", "translateX(-12px)")
                    d3.selectAll("li")
                        .filter(function(otherLanguage) {
                            return d.influenced_by.includes(otherLanguage.name);
                        })
                        .style("transform", "translateX(12px)")
                })
                .on("mouseout", function(event, d){
                    d3.select(this)
                        .style("font-weight", "normal")
                        .style("background-color", "transparent");
                    d3.selectAll("li")
                        .filter(function(otherLanguage) {
                            return d.influences.includes(otherLanguage.name);
                        })
                        .style("transform", "translateX(0px)")
                    d3.selectAll("li")
                        .filter(function(otherLanguage) {
                            return d.influenced_by.includes(otherLanguage.name);
                        })
                        .style("transform", "translateX(0px)")
                });
    });
}
