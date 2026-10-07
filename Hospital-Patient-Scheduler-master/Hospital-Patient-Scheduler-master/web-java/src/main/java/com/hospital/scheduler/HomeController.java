package com.hospital.scheduler;

import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.GetMapping;

@Controller
public class HomeController {

    @GetMapping("/")
    public String home() {
        return "index";
    }

    @GetMapping("/scheduler")
    public String scheduler() {
        return "scheduler";
    }

    @GetMapping("/metrics")
    public String metrics() {
        return "metrics";
    }

    @GetMapping("/gantt")
    public String gantt() {
        return "gantt";
    }

    @GetMapping("/compare")
    public String compare() {
        return "compare";
    }
}
