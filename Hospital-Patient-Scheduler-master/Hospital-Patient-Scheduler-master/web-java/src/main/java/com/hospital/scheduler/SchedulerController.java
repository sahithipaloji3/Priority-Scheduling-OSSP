package com.hospital.scheduler;

import org.springframework.web.bind.annotation.*;

import java.io.*;
import java.util.*;

@RestController
@RequestMapping("/api")
@CrossOrigin
public class SchedulerController {

    private static final String C_SCHEDULER = "./web_scheduler";

    @PostMapping("/schedule")
    public SchedulerResponse schedule(
            @RequestBody SchedulerRequest request) {

        if (request == null ||
                request.getProcesses() == null ||
                request.getProcesses().isEmpty()) {

            throw new IllegalArgumentException(
                    "At least one process is required."
            );
        }

        int algorithm;

        if ("non-preemptive".equalsIgnoreCase(
                request.getAlgorithm())) {

            algorithm = 0;

        } else if ("preemptive".equalsIgnoreCase(
                request.getAlgorithm())) {

            algorithm = 1;

        } else {

            throw new IllegalArgumentException(
                    "Invalid scheduling algorithm."
            );
        }

        try {

            ProcessBuilder builder =
                    new ProcessBuilder(C_SCHEDULER);

            builder.redirectErrorStream(false);

            Process process = builder.start();

            /*
             * Send data to C scheduler.
             */

            try (BufferedWriter writer =
                         new BufferedWriter(
                                 new OutputStreamWriter(
                                         process.getOutputStream()))) {

                writer.write(
                        String.valueOf(
                                request.getProcesses().size()
                        )
                );

                writer.newLine();

                for (ProcessData p :
                        request.getProcesses()) {

                    writer.write(
                            p.getId()
                                    + " "
                                    + p.getArrival()
                                    + " "
                                    + p.getBurst()
                                    + " "
                                    + p.getPriority()
                    );

                    writer.newLine();
                }

                writer.write(
                        String.valueOf(algorithm)
                );

                writer.newLine();

                writer.flush();
            }

            /*
             * Read C scheduler output.
             */

            List<String> outputLines =
                    new ArrayList<>();

            try (BufferedReader reader =
                         new BufferedReader(
                                 new InputStreamReader(
                                         process.getInputStream()))) {

                String line;

                while ((line = reader.readLine())
                        != null) {

                    outputLines.add(line);
                }
            }

            /*
             * Read C scheduler errors.
             */

            StringBuilder errorOutput =
                    new StringBuilder();

            try (BufferedReader reader =
                         new BufferedReader(
                                 new InputStreamReader(
                                         process.getErrorStream()))) {

                String line;

                while ((line = reader.readLine())
                        != null) {

                    errorOutput
                            .append(line)
                            .append("\n");
                }
            }

            int exitCode = process.waitFor();

            if (exitCode != 0) {

                throw new RuntimeException(
                        "C scheduler failed: "
                                + errorOutput
                );
            }

            return parseSchedulerOutput(
                    request,
                    outputLines
            );

        } catch (IOException e) {

            throw new RuntimeException(
                    "Could not start C scheduler. "
                            + "Make sure web_scheduler exists "
                            + "and is executable.",
                    e
            );

        } catch (InterruptedException e) {

            Thread.currentThread().interrupt();

            throw new RuntimeException(
                    "Scheduler process was interrupted.",
                    e
            );
        }
    }


    private SchedulerResponse parseSchedulerOutput(
            SchedulerRequest request,
            List<String> lines) {

        SchedulerResponse response =
                new SchedulerResponse();

        response.setMessage(
                "Scheduling completed successfully"
        );

        response.setAlgorithm(
                request.getAlgorithm()
        );

        response.setProcessCount(
                request.getProcesses().size()
        );

        List<ProcessResult> processResults =
                new ArrayList<>();

        List<GanttSegment> gantt =
                new ArrayList<>();

        boolean readingProcesses = false;
        boolean readingGantt = false;

        for (String line : lines) {

            if (line.startsWith("AVERAGE_WAITING")) {

                response.setAverageWaiting(
                        Double.parseDouble(
                                line.split("\\s+")[1]
                        )
                );

            } else if (line.startsWith(
                    "AVERAGE_TURNAROUND")) {

                response.setAverageTurnaround(
                        Double.parseDouble(
                                line.split("\\s+")[1]
                        )
                );

            } else if (line.startsWith(
                    "AVERAGE_RESPONSE")) {

                response.setAverageResponse(
                        Double.parseDouble(
                                line.split("\\s+")[1]
                        )
                );

            } else if (line.equals(
                    "PROCESS_RESULTS")) {

                readingProcesses = true;
                readingGantt = false;

            } else if (line.equals("GANTT")) {

                readingProcesses = false;
                readingGantt = true;

            } else if (readingProcesses) {

                String[] parts =
                        line.trim().split("\\s+");

                if (parts.length >= 8) {

                    ProcessResult result =
                            new ProcessResult();

                    result.setId(parts[0]);

                    result.setArrival(
                            Integer.parseInt(parts[1])
                    );

                    result.setBurst(
                            Integer.parseInt(parts[2])
                    );

                    result.setPriority(
                            Integer.parseInt(parts[3])
                    );

                    result.setCompletion(
                            Integer.parseInt(parts[4])
                    );

                    result.setTurnaround(
                            Integer.parseInt(parts[5])
                    );

                    result.setWaiting(
                            Integer.parseInt(parts[6])
                    );

                    result.setResponse(
                            Integer.parseInt(parts[7])
                    );

                    processResults.add(result);
                }

            } else if (readingGantt) {

                String[] parts =
                        line.trim().split("\\s+");

                if (parts.length >= 3) {

                    GanttSegment segment =
                            new GanttSegment();

                    segment.setProcessId(parts[0]);

                    segment.setStart(
                            Integer.parseInt(parts[1])
                    );

                    segment.setEnd(
                            Integer.parseInt(parts[2])
                    );

                    gantt.add(segment);
                }
            }
        }

        response.setProcessResults(
                processResults
        );

        response.setGantt(gantt);

        return response;
    }


    /*
     * ==============================
     * REQUEST CLASSES
     * ==============================
     */

    public static class SchedulerRequest {

        private String algorithm;

        private List<ProcessData> processes =
                new ArrayList<>();

        public String getAlgorithm() {
            return algorithm;
        }

        public void setAlgorithm(
                String algorithm) {

            this.algorithm = algorithm;
        }

        public List<ProcessData> getProcesses() {
            return processes;
        }

        public void setProcesses(
                List<ProcessData> processes) {

            this.processes = processes;
        }
    }


    public static class ProcessData {

        private String id;
        private String task;
        private int arrival;
        private int burst;
        private int priority;

        public String getId() {
            return id;
        }

        public void setId(String id) {
            this.id = id;
        }

        public String getTask() {
            return task;
        }

        public void setTask(String task) {
            this.task = task;
        }

        public int getArrival() {
            return arrival;
        }

        public void setArrival(int arrival) {
            this.arrival = arrival;
        }

        public int getBurst() {
            return burst;
        }

        public void setBurst(int burst) {
            this.burst = burst;
        }

        public int getPriority() {
            return priority;
        }

        public void setPriority(int priority) {
            this.priority = priority;
        }
    }


    /*
     * ==============================
     * RESPONSE CLASSES
     * ==============================
     */

    public static class SchedulerResponse {

        private String message;
        private String algorithm;
        private int processCount;

        private double averageWaiting;
        private double averageTurnaround;
        private double averageResponse;

        private List<ProcessResult> processResults =
                new ArrayList<>();

        private List<GanttSegment> gantt =
                new ArrayList<>();

        public String getMessage() {
            return message;
        }

        public void setMessage(String message) {
            this.message = message;
        }

        public String getAlgorithm() {
            return algorithm;
        }

        public void setAlgorithm(String algorithm) {
            this.algorithm = algorithm;
        }

        public int getProcessCount() {
            return processCount;
        }

        public void setProcessCount(int processCount) {
            this.processCount = processCount;
        }

        public double getAverageWaiting() {
            return averageWaiting;
        }

        public void setAverageWaiting(
                double averageWaiting) {

            this.averageWaiting = averageWaiting;
        }

        public double getAverageTurnaround() {
            return averageTurnaround;
        }

        public void setAverageTurnaround(
                double averageTurnaround) {

            this.averageTurnaround =
                    averageTurnaround;
        }

        public double getAverageResponse() {
            return averageResponse;
        }

        public void setAverageResponse(
                double averageResponse) {

            this.averageResponse =
                    averageResponse;
        }

        public List<ProcessResult>
        getProcessResults() {

            return processResults;
        }

        public void setProcessResults(
                List<ProcessResult> processResults) {

            this.processResults =
                    processResults;
        }

        public List<GanttSegment> getGantt() {
            return gantt;
        }

        public void setGantt(
                List<GanttSegment> gantt) {

            this.gantt = gantt;
        }
    }


    public static class ProcessResult {

        private String id;

        private int arrival;
        private int burst;
        private int priority;

        private int completion;
        private int turnaround;
        private int waiting;
        private int response;

        public String getId() {
            return id;
        }

        public void setId(String id) {
            this.id = id;
        }

        public int getArrival() {
            return arrival;
        }

        public void setArrival(int arrival) {
            this.arrival = arrival;
        }

        public int getBurst() {
            return burst;
        }

        public void setBurst(int burst) {
            this.burst = burst;
        }

        public int getPriority() {
            return priority;
        }

        public void setPriority(int priority) {
            this.priority = priority;
        }

        public int getCompletion() {
            return completion;
        }

        public void setCompletion(int completion) {
            this.completion = completion;
        }

        public int getTurnaround() {
            return turnaround;
        }

        public void setTurnaround(int turnaround) {
            this.turnaround = turnaround;
        }

        public int getWaiting() {
            return waiting;
        }

        public void setWaiting(int waiting) {
            this.waiting = waiting;
        }

        public int getResponse() {
            return response;
        }

        public void setResponse(int response) {
            this.response = response;
        }
    }


    public static class GanttSegment {

        private String processId;

        private int start;
        private int end;

        public String getProcessId() {
            return processId;
        }

        public void setProcessId(
                String processId) {

            this.processId = processId;
        }

        public int getStart() {
            return start;
        }

        public void setStart(int start) {
            this.start = start;
        }

        public int getEnd() {
            return end;
        }

        public void setEnd(int end) {
            this.end = end;
        }
    }
}
